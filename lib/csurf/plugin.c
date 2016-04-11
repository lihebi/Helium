#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cs_sdg.h"
#include "cs_pdg.h"
#include "cs_types.h"
#include "cs_utility.h"
#include "cs_pdg_vertex_set.h"

typedef enum {false, true} bool;

void func(char *filename, int line);
void cs_plug_main();

CS_DEFINE_PLUGIN(plugin);

void cs_plug_main()
{
  printf("Starting Slicing Plugin ..\n");
  FILE *f = fopen("input.txt", "rt");

  if (!f) {
    printf("No input.txt\n");
    exit(1);
  };
  char filename[100];


  /*
   * Process for every line in the input.txt file.
   */
  while(fgets(filename, sizeof(filename), f)!=NULL) {
    /* printf("%s\n", filename); */
    char *p = strchr(filename, ':');
    if (p==NULL) {
      printf("NULL");
      exit(0);
    }
    *p = '\0';

    int line = atoi(++p);
    /* printf("============\n"); */

    func(filename, line);
  }
}

/*
 * do the slicing
 */
void func(char *filename, int line) {
/* printf("Filename: %s; Line: %d\n", filename, line); */
  cs_size_t size;
  int i;


  // HEBI: create PDG from SDG => allPdgs

  cs_sdg_pdgs(NULL, 0, &size);
  cs_pdg *allPdgs = (cs_pdg *) malloc(size);
  cs_sdg_pdgs(allPdgs, size, &size);
  int nPdgs = size / sizeof(cs_pdg);
  if (nPdgs==0) {
    printf("no pdgs at all\n");
    return;
  }

  // HEBI: sort
  // cs_pdg_vertex_set_sort(pdgVerticesSet, "", NULL, 0, &size);
  // pdgVerticesListSorted = (cs_pdg_vertex *) malloc(csSize);
  // cs_pdg_vertex_set_sort(pdgVerticesSet, "", pdgVerticesListSorted, csSize, &csSize);

  cs_line thisLine,nextLine;
  cs_sfid thisSfid,nextSfid;

  // HEBI: get PDG for the line => linePdgList[0]
  cs_line resultLine = 0;
  int index=0;
  for (i=0;i<nPdgs;i++) {
    cs_pdg_compilation_unit(allPdgs[i], NULL, 0, &size);
    cs_file_path pdgFileName = (cs_file_path)malloc(size);
    cs_pdg_compilation_unit(allPdgs[i], pdgFileName, size, &size);
    cs_line result = 0;
    if (strcmp(filename, pdgFileName) == 0) {
      cs_pdg_file_line(allPdgs[i], &thisSfid, &thisLine);
      if (thisLine < line && thisLine > resultLine) {
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
    if(thisLine == line) {
      cs_pdg_vertex_set_put(toSliceSet, pdgVerticesList[i]);
      cs_pdg_vertex_characters(pdgVerticesList[i], NULL, 0, &size);
      cs_string s = (cs_string)malloc(size);
      cs_pdg_vertex_characters(pdgVerticesList[i], s, size, &size);
      // 对的呀
      /* printf("Vertex content: %s\n", s); */
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

  if (nVertices==0) return;
  if (nVertices > 10000) return;
  printf("Slice set size: %d\n", nVertices);
  FILE *fp = fopen("result.txt", "a");
  fprintf(fp, "=======\n");
  fprintf(fp, "slice criteria: %s:%d\n", filename, line);
  // HEBI: map the sliced vertex to line number
  for(i=0;i<nVertices;i++) {
    int a;
    cs_pdg_vertex_file_line(slicedList[i], &thisSfid, &thisLine);
    cs_file_get_include_name(thisSfid, NULL, 0, &size);
    cs_file_path vertexFileName = (cs_file_path) malloc(size);
    cs_file_get_include_name(thisSfid, vertexFileName, size, &size);
    if (strstr(vertexFileName, "codesurfer") == NULL) {
      fprintf(fp, "%s\t%d\n", vertexFileName, thisLine);
      /* printf("%s\t%d\n", vertexFileName, thisLine); */
    }
  }
  fclose(fp);
}
