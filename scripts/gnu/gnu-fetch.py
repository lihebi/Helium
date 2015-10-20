#!/usr/bin/env python3
import sys

"""
first GNU: alive.git
last GNU: gnuzilla.git
"""

html_doc = open(sys.argv[1]).read()
from bs4 import BeautifulSoup
soup = BeautifulSoup(html_doc, "html.parser")

for repo in soup.find_all('td', "sublevel-repo"):
    print("git://git.sv.gnu.org/"+repo.text)
