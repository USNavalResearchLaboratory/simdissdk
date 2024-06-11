#***************************************************************************
#****                                                                  *****
#****                   Classification: UNCLASSIFIED                   *****
#****                    Classified By:                                *****
#****                    Declassify On:                                *****
#****                                                                  *****
#***************************************************************************
#
#
# Developed by: Naval Research Laboratory, Tactical Electronic Warfare Div.
#               EW Modeling & Simulation, Code 5773
#               4555 Overlook Ave.
#               Washington, D.C. 20375-5339
#
# License for source code is in accompanying LICENSE.txt file. If you did
# not receive a LICENSE.txt with this code, email simdis@us.navy.mil.
#
# The U.S. Government retains all rights to use, duplicate, distribute,
# disclose, or release this software.
#
#
import argparse
import atexit
import json
import os
import os.path
import shutil
import subprocess
import sys
import urllib.request
from datetime import datetime
from PIL import Image

##########################################################################
#
# Downloads NWS data from NOAA GRIB service.  Inspired by a set of scripts
# from https://github.com/mapbox/webgl-wind
#
# Requirements:
# * Python 3.6+
# * Access to the NOAA NWS GRIB filter (https://nomads.necp.noa.gov)
# * Python PIL library
#    * pip install pillow
# * grib_set and grib_dump utility applications
#    * This script has not been tested on Windows due to limited availability of these executables.
#    * These are provided on CentOS and RHEL in the EPEL repository.  See OS-specific instructions for activating EPEL.
#    * sudo yum install eccodes      # also available in grib_api
#
##########################################################################

today = datetime.today()
DATE = f"{today.year}{str(today.month).zfill(2)}{str(today.day).zfill(2)}"
HOUR = "00"   # 00, 06, 12, 18
RESOLUTION = "1p00"   # 0p25, 0p50 or 1p00
# Bounding box from mapbox example does not appear to work

U_QUERY = "&var_UGRD=on"
V_QUERY = "&var_VGRD=on"

BASE_URL = f"https://nomads.ncep.noaa.gov"
LEVEL="lev_10_m_above_ground=on"

# Parse input arguments
parser = argparse.ArgumentParser(description='Generate wind velocity from NWS GRIB Service')
parser.add_argument("-d", "--date", default=DATE, help="Date, in the format YYYYMMDD such as 20201110")
parser.add_argument("-t", "--time", default=HOUR, help="UTC Hour, either 00, 06, 12, or 18")
parser.add_argument("-r", "--resolution", default=RESOLUTION, help="Resolution, either 1p00, 0p50, or 0p25")
parser.add_argument("-e", "--earth", help="Write an earth file")
parser.add_argument("-x", "--exclude-base-map", action="store_true", help="Earth file written will not include base map.  Implies -e")
parser.add_argument("-p", "--points", action="store_true", help="Earth file written will show points instead of sprites.  Implies -e")
parser.add_argument("-s", "--sprites", action="store_true", help="Earth file written will show sprites instead of points.  Implies -e")
parser.add_argument("-q", "--quiet", action="store_true", help="Do not print success messages")
parser.add_argument("file", nargs="?", help="Output file")
args = parser.parse_args()

# Get the output file name
pngFile = f"nws_{args.date}_{args.time}z_{args.resolution}.png" if args.file is None else args.file
QUERY = f"/cgi-bin/filter_gfs_{args.resolution}.pl?file=gfs.t{args.time}z.pgrb2.{args.resolution}.f000&{LEVEL}&dir=%2Fgfs.{args.date}%2F${args.time}"

# Download the URL to a local file
def download(url, filename):
	with urllib.request.urlopen(url) as response, open(filename, 'wb') as out_file:
		shutil.copyfileobj(response, out_file)

# Searches through the JSON content returned from grib_dump
def findMessageValue(j, key):
	for dicts in j['messages'][0]:
		if dicts['key'] == key:
			return dicts['value']
	return None

# Downloads the file from NOAA, normalizes it, dumps JSON, then loads and processes the JSON, returning
# a dict that contains relevant content for image generation.
def downloadAndPrep(url, tmpFile, quiet):
	try:
		if not quiet:
			print(f"Downloading: {url}")
		download(url, tmpFile)
	except urllib.error.HTTPError as exception:
		print(f"Fatal error downloading {url}: {exception}\n", file=sys.stderr)
		raise exception
	# Remove the temporary file when we exit
	atexit.register(os.remove, tmpFile)

	# Running grib_set here will normalize the data
	subprocess.run(["grib_set", "-r", "-s", "packingType=grid_simple", tmpFile, tmpFile])
	# Dump out the data in JSON format
	jsonBytes = subprocess.Popen(["grib_dump", "-j", tmpFile], stdout=subprocess.PIPE).communicate()[0]

	# Load and parse the JSON
	j = json.loads(jsonBytes)
	content = {}
	content['width'] = findMessageValue(j, 'Ni')
	# Note that Mapbox example subtracts one from Nj for the height; we do not do so
	content['height'] = findMessageValue(j, 'Nj')
	content['minimum'] = findMessageValue(j, 'minimum')
	content['maximum'] = findMessageValue(j, 'maximum')
	content['values'] = findMessageValue(j, 'values')
	return content

uFile = downloadAndPrep(BASE_URL + QUERY + U_QUERY, pngFile + ".u.grb", args.quiet)
vFile = downloadAndPrep(BASE_URL + QUERY + V_QUERY, pngFile + ".v.grb", args.quiet)

# Extract values to convenient variables
width = uFile['width']
height = uFile['height']
uMin = uFile['minimum']
uMax = uFile['maximum']
uValues = uFile['values']
vMin = vFile['minimum']
vMax = vFile['maximum']
vValues = vFile['values']

# Normalize the output.  We could write with more precision by balancing on the actual
# minimum and maximum, but instead we'll use a normalized output and get less precision,
# with a benefit of being able to use the same scale for velocity.
uMin = vMin = -25.0
uMax = vMax = 25.0
uRange = uMax - uMin
vRange = vMax - vMin

pixels = []
for y in range(0, height):
	for x in range(0, width):
		# The data starts at longitude 0 (Greenwich).  So we add half the width in order
		# to instead "start" at the dateline, marching east.
		pos = int(y * width + ((x + width / 2) % width))
		# Reproject to [0,255]
		u = 255 * (uValues[pos] - uMin) / uRange
		v = 255 * (vValues[pos] - vMin) / vRange
		pixels.append((int(max(0, min(255, u))), int(max(0, min(255, v))), 0))
img = Image.new("RGB", (width, height))
img.putdata(pixels)
img.save(pngFile)

if not args.quiet:
	print(f"Wrote: {pngFile}")

# Write out the preamble to a .earth file, including the XML header, map, version, and options tags
def writeEarthPreamble(file):
	file.write("""<?xml version="1.0" ?>
<map>
  <version>3</version>
  <options name="SIMDIS Default">
    <type>geocentric</type>
    <profile>
      <srs>wgs84</srs>
    </profile>
    <terrain>
      <first_level>0</first_level>
      <driver>rex</driver>
      <first_lod>0</first_lod>
    </terrain>
  </options>
""")

# Write out the earth base layers from default earth file
def writeEarthBaseLayers(file):
	file.write("""  <MBTilesElevation name="Ni`ihau">
    <nodata_value>0</nodata_value>
    <compute_levels>false</compute_levels>
    <cache_policy>
      <usage>no_cache</usage>
    </cache_policy>
    <visible>true</visible>
    <no_data_value>0</no_data_value>
    <min_valid_value>0.001</min_valid_value>
    <filename>USGS/Niihau_USGS_NED_10m_L12.mbtiles</filename>
  </MBTilesElevation>
  <MBTilesElevation name="Kaua`i">
    <nodata_value>0</nodata_value>
    <compute_levels>false</compute_levels>
    <cache_policy>
      <usage>no_cache</usage>
    </cache_policy>
    <visible>true</visible>
    <no_data_value>0</no_data_value>
    <min_valid_value>0.001</min_valid_value>
    <filename>USGS/Kauai_USGS_NED_10m_L12.mbtiles</filename>
  </MBTilesElevation>
  <MBTilesImage name="Whole Earth">
    <compute_levels>false</compute_levels>
    <cache_policy>
      <usage>no_cache</usage>
    </cache_policy>
    <visible>true</visible>
    <opacity>1</opacity>
    <filename>WorldImagery/TE2_480_srtm30_plus_bathy_hill_0-5.mbtiles</filename>
  </MBTilesImage>
  <MBTilesImage name="Kaua`i Ni`ihau">
    <compute_levels>false</compute_levels>
    <cache_policy>
      <usage>no_cache</usage>
    </cache_policy>
    <visible>true</visible>
    <opacity>1</opacity>
    <filename>WorldImagery/Kauai_wv3_Niihau_wv2_8_12_webp.mbtiles</filename>
  </MBTilesImage>
""")

# Write out the VelocityParticleImage to the earth file
def writeEarthVelocity(file, pngFile, useSprites):
	file.write(f"  <VelocityParticleImage name=\"{pngFile}\">\n")
	file.write(f"    <opacity>0.8</opacity>\n")
	file.write(f"    <particle_dimension>{512 if useSprites else 1024}</particle_dimension>\n")
	file.write(f"    <speed_factor>{0.25 if useSprites else 0.5}</speed_factor>\n")
	file.write(f"    <point_size>{6 if useSprites else 2}</point_size>\n")
	if useSprites:
		file.write("    <sprite_uri>WindSprite.png</sprite_uri>\n")
	file.write(f"    <uri>{pngFile}</uri>\n")
	file.write(f"  </VelocityParticleImage>\n")

# Writes out the whole earth file with options
def writeEarth(file, pngFile, useSprites, excludeBaseMap):
	writeEarthPreamble(file)
	if not excludeBaseMap:
		writeEarthBaseLayers(file)
	writeEarthVelocity(file, pngFile, useSprites)
	file.write("</map>\n")

# Determine if the arguments are set up to request a .earth file
if args.earth is not None or args.points or args.sprites or args.exclude_base_map:
	# Get the output filename for the earth file
	earthFile = os.path.splitext(pngFile)[0] + ".earth"
	if args.earth is not None:
		earthFile = args.earth

	# Write the earth file out
	with open(earthFile, 'w') as file:
		writeEarth(file, pngFile, args.sprites or not args.points, args.exclude_base_map)
	if not args.quiet:
		print("Wrote: " + earthFile)
