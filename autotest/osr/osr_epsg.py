#!/usr/bin/env python
###############################################################################
# $Id: osr_pm.py 11065 2007-03-24 09:35:32Z mloskot $
#
# Project:  GDAL/OGR Test Suite
# Purpose:  Test aspects of EPSG code lookup.
# Author:   Frank Warmerdam <warmerdam@pobox.com>
# 
###############################################################################
# Copyright (c) 2007, Frank Warmerdam <warmerdam@pobox.com>
# 
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.
###############################################################################

import os
import sys

sys.path.append( '../pymod' )

import gdaltest
import osr
import string

###############################################################################
#	Verify that EPSG:26591 picks up the entry from the pcs.override.csv
# 	file with the adjusted central_meridian.

def osr_epsg_1():
    
    srs = osr.SpatialReference()
    srs.ImportFromEPSG( 26591 )

    if abs(srs.GetProjParm('central_meridian') - -3.4523333333333) > 0.000005:
        gdaltest.post_reason( 'Wrong central meridian, override missed?' )
        return 'fail'
    
    return 'success'

###############################################################################
#	Check that EPSG:4312 lookup has the towgs84 values set properly
#	from gcs.override.csv.

def osr_epsg_2():
    
    srs = osr.SpatialReference()
    srs.ImportFromEPSG( 4312 )

    if abs(float(srs.GetAttrValue( 'TOWGS84', 6)) \
           - 2.4232) > 0.0005:
        gdaltest.post_reason( 'Wrong TOWGS84, override missed?' )
        return 'fail'
    
    return 'success'

###############################################################################

gdaltest_list = [ 
    osr_epsg_1,
    osr_epsg_2,
    None ]

if __name__ == '__main__':

    gdaltest.setup_run( 'osr_epsg' )

    gdaltest.run_tests( gdaltest_list )

    gdaltest.summarize()

