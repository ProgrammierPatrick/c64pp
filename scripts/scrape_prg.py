#!/usr/bin/env python3
from distutils import filelist
import requests
from tabulate import tabulate
from zipfile import ZipFile
from rarfile import RarFile
import json
import re
import io

data = []

text = input("Search: ")

ignored_files = ["JPG", "GIF", "ENT", "XML", "ITE", "PNG"]

##############
# CSDb SCRAPER
print (f"[*] request search results... (https://csdb.dk/search/?seinsel=releases&search={text}&all=1)")
search = requests.get(f"https://csdb.dk/search/?seinsel=releases&search={text}&all=1")
regex = re.compile(r'</a><a href="(/release/\?id=\d+)">([^<>]+)</a> \(([^\(\)]+)\) by  <a href="[^"]+"><font color=#[\da-f]+>([^<]+)</font></a>')
for match in regex.finditer(search.text):
    url = f"https://csdb.dk{match.group(1)}"
    name = match.group(2)
    type = match.group(3)
    cracker = match.group(4)

    print (f"[*] request result page for {name} by {cracker}... ({url})")
    page = requests.get(url)
    download_match = re.search(r'<a href="download.php\?id=\d+">(.+)</a> \(downloads: (\d+)\)', page.text)
    dl_url = download_match.group(1)
    dl_count = download_match.group(2)
    format = dl_url[-3:].upper()

    try:
        if format == ".GZ":
            format = dl_url[-6:-3]
        if format == "ZIP":
            print (f"[*] download zip file to check file type... ({dl_url})")
            zip_file = io.BytesIO(requests.get(dl_url).content)
            zip = ZipFile(zip_file, 'r')
            formats = set()
            for file in zip.namelist():
                formats.add(file[-3:].upper())
            format = ",".join(formats)
        if format == "RAR":
            print (f"[*] download rar file to check file type... ({dl_url})")
            zip_file = io.BytesIO(requests.get(dl_url).content)
            zip = RarFile(zip_file, 'r')
            formats = set()
            for file in zip.namelist():
                formats.add(file[-3:].upper())
            format = ",".join(formats)
    except Exception as e:
        print(e)

    data.append({ "type": type, "name": name, "cracker": cracker, "url": url, "dl_url": dl_url, "dl_count": dl_count, "format": format })

###################
# PLANETEMU SCRAPER
url = f"https://www.planetemu.net/roms/commodore-c64-games-prg?page={text[0].upper()}"
print (f"[*] request search results... ({url})")
search = requests.get(url)
regex = re.compile(r'<a href="(/rom/commodore-c64-games-prg/[^"]+)">([^<]+)</a>')
for match in regex.finditer(search.text):
    url = f"https://www.planetemu.net{match.group(1)}"
    name = match.group(2).strip()

    match = True
    for s in text.split():
        if not s.lower() in name.lower():
            match = False
    if not match:
        continue

    print (f"[*] request result page for {name}... ({url})")
    page = requests.get(url)

    filename = re.search(r'<td class="detail-left">Archive</td>[^<]*<td class="detail-right">([^>]+)</td>', page.text).group(1)
    format = filename[-3:].upper()

    try:
        if format == "ZIP":
            id = re.search(r'name="id" value="(\d+)"', page.text).group(1)
            print (f"[*] download zip file to check file type... (post : https://www.planetemu.net/php/roms/download.php?id={id})")
            post_resp = requests.post("https://www.planetemu.net/php/roms/download.php", {"id": id})
            zip = ZipFile(io.BytesIO(post_resp.content))
            formats = set()
            for file in zip.namelist():
                if not file[-3:].upper() in ignored_files:
                    formats.add(file[-3:].upper())
            format = ",".join(formats)
        if format == "RAR":
            id = re.search(r'name="id" value="(\d+)"', page.text).group(1)
            print (f"[*] download rar file to check file type... (post : https://www.planetemu.net/php/roms/download.php?id={id})")
            post_resp = requests.post("https://www.planetemu.net/php/roms/download.php", {"id": id})
            zip = RarFile(io.BytesIO(post_resp.content))
            formats = set()
            for file in zip.namelist():
                if not file[-3:].upper() in ignored_files:
                    formats.add(file[-3:].upper())
            format = ",".join(formats)
    except Exception as e:
        print(e)

    data.append({"type": "C64 Crack", "name": name, "cracker": "", "url": url, "dl_url": "", "dl_count": "", "format": format})

################################################
# ARCHIVE.ORG SOFTWARE LIBRARY C64 GAMES SCRAPER
url = f"https://archive.org/advancedsearch.php?q=title:({text})%20AND%20collection:(softwarelibrary_c64_games)&fl[]=downloads&fl[]=identifier&fl[]=title&output=json"
print (f"[*] request search results... ({url})")
search = json.loads(requests.get(url).text)["response"]["docs"]
for s in search:
    id = s["identifier"]
    url = f"https://archive.org/details/{id}"
    name = s["title"]
    dl_count = s["downloads"]

    print (f"[*] request metadata page for {name}... (https://archive.org/metadata/{id}/files)")
    meta = json.loads(requests.get(f"https://archive.org/metadata/{id}/files").text)["result"]
    formats = set()
    for m in meta:
        if not m["name"][-3:].upper() in ignored_files:
            formats.add(m["name"][-3:].upper())
    format = ",".join(formats)

    data.append({"type": "C64 Crack", "name": name, "cracker": "", "url": url, "dl_url": "", "dl_count": dl_count, "format": format})



crack_prg_data = [{"name":"name", "cracker":"cracker", "downloads":"downloads","url":"url"}]
crack_data = [{"name":"name", "format": "format", "cracker":"cracker", "downloads":"downloads","url":"url"}]
other_data = [{"name":"name", "format": "format", "type":"type", "cracker":"cracker", "downloads":"downloads","url":"url"}]
for d in data:
    if d["type"] == "C64 Crack":
        if "PRG" in d["format"]:
            crack_prg_data.append({"name": d["name"], "cracker": d["cracker"], "downloads": d["dl_count"], "url": d["url"]})
        else:
            crack_data.append({"name": d["name"], "format": d["format"], "cracker": d["cracker"], "downloads": d["dl_count"], "url": d["url"]})
    else:
        other_data.append({"name": d["name"], "type": d["type"], "format": d["format"], "cracker": d["cracker"], "downloads": d["dl_count"], "url": d["url"]})

print()
if len(crack_prg_data) > 0:
    print("CRACK RELEASES IN PRG FORMAT")
    print(tabulate(crack_prg_data, headers='firstrow', tablefmt='pretty'))
if len(crack_data) > 0:
    print("OTHER CRACK RELEASES")
    print(tabulate(crack_data, headers='firstrow', tablefmt='pretty'))
if len(other_data) > 0:
    print("OTHER")
    print(tabulate(other_data, headers='firstrow', tablefmt='pretty'))