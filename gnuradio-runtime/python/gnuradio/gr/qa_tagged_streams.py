#!/usr/bin/env python
#
# Copyright 2014 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# GNU Radio is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# GNU Radio is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Radio; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
#

from gnuradio import gr, gr_unittest
try:
    from gnuradio import blocks
    HAVE_BLOCKS = True
except ImportError:
    HAVE_BLOCKS = False

class test_tagged_streams (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def test_001(self):
        pass
        #tag_name = "ts_last"
        #packet_len = 4
        #data = range(2*packet_len)
        #sink = blocks.vector_sink_b()
        #self.tb.connect(
                #blocks.vector_source_b(data),
                #blocks.stream_to_tagged_stream(gr.sizeof_char, 1, packet_len, tag_name),
                #sink
        #)
        #self.tb.run()
        #packets = gr.tagged_streams.data_to_packets(sink.data(), sink.tags(), tag_name)
        #self.assertEqual(list(packets[0]), data[0:packet_len])
        #self.assertEqual(list(packets[1]), data[packet_len:2*packet_len])


if __name__ == '__main__':
    if HAVE_BLOCKS:
        gr_unittest.run(test_tagged_streams, "test_tagged_streams.xml")


