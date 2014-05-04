/* -*- c++ -*- */
/*
 * Copyright 2013-2014 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <boost/format.hpp>
#include <gnuradio/tagged_stream_block.h>

namespace gr {

  tagged_stream_block::tagged_stream_block(const std::string &name,
                                           io_signature::sptr input_signature,
                                           io_signature::sptr output_signature,
                                           const std::string &tsb_key)
    : block(name, input_signature, output_signature),
      d_tsb_key(pmt::string_to_symbol(tsb_key)),
      d_input_buffer_ready(input_signature->min_streams(), false),
      d_n_input_items_reqd(input_signature->min_streams(), 0),
      d_tsb_key_str(tsb_key)
  {
  }

  // This is evil hackery: We trick the scheduler into creating the right number of input items
  void
  tagged_stream_block::forecast(int noutput_items,
                                gr_vector_int &ninput_items_required)
  {
    unsigned ninputs = ninput_items_required.size();
    for(unsigned i = 0; i < ninputs; i++) {
      if (i < d_n_input_items_reqd.size() && d_n_input_items_reqd[i] != 0) {
        ninput_items_required[i] = d_n_input_items_reqd[i];
      }
      else {
        // If there's no item, there's no tag--so there must at least be one!
        ninput_items_required[i] = 1;
      }
    }
  }

  int
  tagged_stream_block::calculate_output_stream_length(const gr_vector_int &ninput_items)
  {
    int noutput_items = *std::max_element(ninput_items.begin(), ninput_items.end());
    return (int)std::floor(relative_rate() * noutput_items + 0.5);
  }

  void
  tagged_stream_block::update_length_tags(int n_produced, int n_ports)
  {
    for(int i = 0; i < n_ports; i++) {
      add_item_tag(
	  i,
	  nitems_written(i) + n_produced - 1,
	  d_tsb_key,
	  pmt::PMT_T
      );
    }
    return;
  }

  int
  tagged_stream_block::general_work(int noutput_items,
                                    gr_vector_int &ninput_items,
                                    gr_vector_const_void_star &input_items,
                                    gr_vector_void_star &output_items)
  {
    if(d_tsb_key_str.empty()) {
      return work(noutput_items, ninput_items, input_items, output_items);
    }

    std::cout << "general_work()" << std::endl;
    // - Check for tsb tag
    // YES:
    //	  - check if output buffer is large enough, if necessary increase min output size
    //	  - call work
    //	  - consume, produce
    //	  - add new tsb tag
    // NO:
    //    - Set the req'd min to what we have + 1 (?)
    //    - Return

    // 1) Search for packet boundary tags
    bool all_tags_found = true;
    std::vector<tag_t> tags;
    for(size_t i = 0; i < input_items.size(); i++) {
      if (d_input_buffer_ready[i]) {
	continue;
      }
      get_tags_in_window(tags, i, 0, ninput_items[i], d_tsb_key);
      if (not tags.empty()) {
	std::sort(tags.begin(), tags.end(), tag_t::offset_compare);
	d_n_input_items_reqd[i] = tags[0].offset - nitems_read(i) + 1;
	d_input_buffer_ready[i] = true;
	remove_item_tag(i, tags[0]);
      } else {
	all_tags_found = false;
	// Discuss: Is there a better way to do this?
	d_n_input_items_reqd[i] = ninput_items[i] + 1;
      }
    }
    if (not all_tags_found) {
      return 0;
    }

    // 2) OK, all tags are there. Check output buffers.
    int min_output_size = calculate_output_stream_length(d_n_input_items_reqd);
    if(noutput_items < min_output_size) {
      set_min_noutput_items(min_output_size);
      return 0;
    }
    set_min_noutput_items(1);

    // 3) Buffers are all ready. Call work etc.
    // WORK CALLED HERE //
    int n_produced = work(noutput_items, d_n_input_items_reqd, input_items, output_items);
    //////////////////////

    if(n_produced == WORK_DONE) {
      return n_produced;
    }
    for(int i = 0; i < (int) d_n_input_items_reqd.size(); i++) {
      consume(i, d_n_input_items_reqd[i]);
    }
    if (n_produced > 0) {
      update_length_tags(n_produced, output_items.size());
    } else if (n_produced == WORK_CALLED_PRODUCE) {
      update_length_tags(-1, output_items.size());
    }

    d_input_buffer_ready.assign(input_items.size(), false);
    d_n_input_items_reqd.assign(input_items.size(), 0);

    return n_produced;
  }  // general_work()

}  /* namespace gr */
