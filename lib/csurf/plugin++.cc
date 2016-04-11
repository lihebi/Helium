#include <iostream>
#include <fstream>
#include <algorithm>
#include <cassert>
#include <cstring>
/**
 * C++ version for the slicing plugin
 */

#include "cs_sdg.h"
#include "cs_pdg.h"
#include "cs_types.h"
#include "cs_utility.h"
#include "cs_pdg_vertex_set.h"


void do_slice(std::string filename, int linum);
void cs_plug_main();
CS_DEFINE_PLUGIN(plugin);

void cs_plugin_main() {
  std::cout << "Starting slicing plugin .."  << "\n";
  std::ifstream is;
  is.open("input.txt");
  if (is.is_open()) {
    std::string line;
    while (std::getline(is, line)) {
      assert(std::count(line.begin(), line.end(), ':') == 1);
      std::string filename = line.substr(0, line.find(':'));
      std::string linum_str = line.substr(line.find(':')+1);
      int linum = atoi(linum_str.c_str());
      do_slice(filename, linum);
    }
  }
}

void do_slice(std::string filename, int linum) {
  std::cout << "doing slicing on: " << filename << ":" << linum  << "\n";
  cs_size_t size;
  int i;
  // HEBI: create PDG from SDG => allPdgs
  cs_sdg_pdgs(NULL, 0, &size);
  cs_pdg *allPdgs = (cs_pdg *) malloc(size);
  cs_sdg_pdgs(allPdgs, size, &size);
  int nPdgs = size / sizeof(cs_pdg);
  if (nPdgs==0) {
    std::cerr << "no pdgs at all\n";
    return;
  }
  // HEBI: get PDG for the line => linePdgList[0]
  cs_line thisLine,nextLine;
  cs_sfid thisSfid,nextSfid;
  cs_line resultLine = 0;
  int index=0;
  for (i=0;i<nPdgs;i++) {
    cs_pdg_compilation_unit(allPdgs[i], NULL, 0, &size);
    cs_file_path pdgFileName = (cs_file_path)malloc(size);
    cs_pdg_compilation_unit(allPdgs[i], pdgFileName, size, &size);
    cs_line result = 0;
    if (filename == pdgFileName) {
      cs_pdg_file_line(allPdgs[i], &thisSfid, &thisLine);
      if (thisLine < linum && thisLine > resultLine) {
        resultLine = thisLine;
        index = i;
      }
    }
  }
  // printf("Line: %d\n", resultLine);
  if (resultLine==0) {
    return;
  }
  cs_pdg_compilation_unit(allPdgs[index], NULL, 0, &size);
  cs_file_path pdgFileName = (cs_file_path)malloc(size);
  cs_pdg_compilation_unit(allPdgs[index], pdgFileName, size, &size);
  cs_pdg_file_line(allPdgs[index], &thisSfid, &thisLine);

  cs_pdg *linePdgList;
  cs_sf *sfName;
  cs_sfid_sf(thisSfid, &sfName);
  cs_sf_line_pdgs(sfName, thisLine, cs_true, NULL, 0, &size);
  linePdgList = (cs_pdg*) malloc(size);
  cs_sf_line_pdgs(sfName, thisLine, cs_true, linePdgList, size, &size);
  // HEBI: in this PDG, find the vertex for the line => toSliceSet(size 1)
  cs_const_pdg_vertex_set pdgVertexSet;
  cs_pdg_vertices(linePdgList[0], &pdgVertexSet);
  cs_pdg_vertex_set_to_list(pdgVertexSet, NULL, 0, &size);
  cs_pdg_vertex *pdgVerticesList = (cs_pdg_vertex*) malloc(size);
  cs_pdg_vertex_set_to_list(pdgVertexSet, pdgVerticesList, size, &size);
  cs_size_t nVertices = size / sizeof(cs_pdg_vertex);

  // printf("Vertex number in this PDG containing line: %d\n", nVertices);

  cs_pdg_vertex_set toSliceSet, slicedSet;
  cs_pdg_vertex_set_create_default(&toSliceSet);
  for(i =0; i < nVertices; i++) {
    cs_pdg_vertex_file_line(pdgVerticesList[i], &thisSfid, &thisLine);
    if(thisLine == linum) {
      cs_pdg_vertex_set_put(toSliceSet, pdgVerticesList[i]);
      cs_pdg_vertex_characters(pdgVerticesList[i], NULL, 0, &size);
      cs_string s = (cs_string)malloc(size);
      cs_pdg_vertex_characters(pdgVerticesList[i], s, size, &size);
      printf("Vertex content: %s\n", s);
      break;
    }
  }
  cs_result result;
  result = cs_pdg_vertex_set_to_list(toSliceSet, NULL, 0, &size);
  // printf("toSliceSet size: %d\n", size / sizeof(cs_pdg_vertex));

  // HEBI: do slice on the vertex of line => slicedSet
  result = cs_s_slice(toSliceSet, cs_pdg_edge_kind_datacontrol, &slicedSet);
  result = cs_pdg_vertex_set_to_list(slicedSet, NULL, 0, &size);

  cs_pdg_vertex *slicedList = (cs_pdg_vertex*)malloc(size);
  result = cs_pdg_vertex_set_to_list(slicedSet, slicedList, size, &size);
  nVertices = size / sizeof(cs_pdg_vertex);

  /**
   * Outputing
   */

  printf("Slice set size: %d\n", nVertices);
  FILE *fp = fopen("result.txt", "w");
  // HEBI: map the sliced vertex to line number
  for(i=0;i<nVertices;i++) {
    cs_pdg_vertex_file_line(slicedList[i], &thisSfid, &thisLine);
    cs_file_get_include_name(thisSfid, NULL, 0, &size);
    cs_file_path vertexFileName = (cs_file_path) malloc(size);
    cs_file_get_include_name(thisSfid, vertexFileName, size, &size);
    if (strstr(vertexFileName, "codesurfer") == NULL) {
      fprintf(fp, "%s\t%d\n", vertexFileName, thisLine);
    }
  }
  fclose(fp);
}
