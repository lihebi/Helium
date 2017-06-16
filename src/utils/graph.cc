#include "helium/utils/graph.h"
#include "helium/parser/visitor.h"
#include "helium/utils/dot.h"

namespace hebigraph {
  template <typename T> std::string Graph<T>::visualize(std::string (*labelFunc)(T)) {

    DotGraph dotgraph;
    std::map<T, int> IDs;
    int ID=0;
    {
      VertexIter begin,end;
      boost::tie(begin, end) = vertices(g);
      for (VertexIter it=begin;it!=end;++it) {
        Vertex v = *it;
        T t = Vertex2Node[v];
        std::string label = labelFunc(t);
        dotgraph.AddNode(std::to_string(ID), label);
        IDs[t]=ID;
        ID++;
      }
    }
    {
      EdgeIter begin,end;
      boost::tie(begin, end) = edges(g);
      for (EdgeIter it=begin;it!=end;++it) {
        Edge e = *it;
        Vertex src = source(e, g);
        Vertex dst = target(e, g);
        T srcT = Vertex2Node[src];
        T dstT = Vertex2Node[dst];
        std::string label = get(edge_label_t(), g, e);
        dotgraph.AddEdge(std::to_string(IDs[srcT]), std::to_string(IDs[dstT]), label);
      }
    }
    std::string dotstring = dotgraph.dump();
    return visualize_dot_graph(dotstring);
  }

  template <typename T> void Graph<T>::connect(Graph gother, std::string label) {
    // 1. record exit nodes and other's entry
    std::set<T> orig_exit = getExit();
    std::set<T> other_entry = gother.getEntry();
    // 2. merge gother
    merge(gother);
    // 3. Add edge from this exit to gother's entry
    for (T x : orig_exit) {
      for (T y : other_entry) {
        addEdge(x, y, label);
      }
    }
  }

  template <typename T> void Graph<T>::connect(Graph gother, T from, std::string label) {
    std::set<T> other_entry = gother.getEntry();
    merge(gother);
    for (T to : other_entry) {
      addEdge(from, to, label);
    }
  }


  template <typename T> void Graph<T>::merge(Graph gother) {
    // get the nodes
    // create nodes
    {
      VertexIter begin,end;
      boost::tie(begin,end) = vertices(gother.g);
      for (VertexIter it=begin;it!=end;++it) {
        Vertex v = *it;
        addNode(gother.Vertex2Node[v]);
      }
    }
    {
      // get edges
      // create edges
      EdgeIter begin,end;
      boost::tie(begin,end) = edges(gother.g);
      for (EdgeIter it=begin;it!=end;++it) {
        Edge e = *it;
        Vertex src = source(e, gother.g);
        Vertex dst = target(e, gother.g);
        // Also I need to transfer the edge label
        std::string label = get(edge_label_t(), gother.g, e);
        addEdge(gother.Vertex2Node[src], gother.Vertex2Node[dst], label);
      }
    }
  }

  template <typename T> std::set<T> Graph<T>::getEntry() {
    std::set<T> ret;
    VertexIter begin,end;
    boost::tie(begin,end) = vertices(g);
    for (VertexIter it=begin;it!=end;++it) {
      Vertex v = *it;
      if (in_degree(v, g) == 0) ret.insert(Vertex2Node[v]);
    }
    return ret;
  }
  template <typename T> std::set<T> Graph<T>::getExit() {
    std::set<T> ret;
    VertexIter begin,end;
    boost::tie(begin,end) = vertices(g);
    for (VertexIter it=begin;it!=end;++it) {
      Vertex v = *it;
      if (out_degree(v, g) == 0) ret.insert(Vertex2Node[v]);
    }
    return ret;
  }
  

  
  template class hebigraph::Graph<v2::CFGNode*>;
}
