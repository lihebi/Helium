#include "helium/utils/Graph.h"
#include "helium/parser/Visitor.h"
#include "helium/utils/Dot.h"
#include "helium/utils/AggGraph.h"
#include "helium/utils/FSUtils.h"

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

  template <typename T> std::string Graph<T>::visualizeAgg(std::string (*labelFunc)(T)) {
    AggGraph agg;
    std::map<T, int> IDs;
    int ID=0;
    {
      VertexIter begin,end;
      boost::tie(begin, end) = vertices(g);
      for (VertexIter it=begin;it!=end;++it) {
        Vertex v = *it;
        T t = Vertex2Node[v];
        std::string label = labelFunc(t);
        agg.addNode(std::to_string(ID), label);
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
        agg.addEdge(std::to_string(IDs[srcT]), std::to_string(IDs[dstT]), label);
      }
    }
    std::string aggstring = agg.dump();
    std::string dir = utils::create_tmp_dir();
    std::string agg_filename = dir + "/" + "out" + ".ggx";
    utils::write_file(agg_filename, aggstring);
    return agg_filename;
  }
  

  template <typename T> void Graph<T>::merge(Graph<T> &rhs) {
    {
      VertexIter begin,end;
      boost::tie(begin, end) = vertices(rhs.g);
      for (VertexIter it=begin;it!=end;++it) {
        Vertex v = *it;
        T node = rhs.Vertex2Node[v];
        this->addNode(node);
      }
    }
    {
      EdgeIter begin,end;
      boost::tie(begin,end) = edges(rhs.g);
      for (EdgeIter it=begin;it!=end;++it) {
        Edge e = *it;
        Vertex src = source(e, rhs.g);
        Vertex dst = target(e, rhs.g);
        // Also I need to transfer the edge label
        std::string label = get(edge_label_t(), rhs.g, e);
        this->addEdge(rhs.Vertex2Node[src], rhs.Vertex2Node[dst], label);
      }
    }
  }


  /**
   * Modifying this to use manually defined entries and exits
   */
  // template <typename T> std::set<T> Graph<T>::getEntry() {
  //   std::set<T> ret;
  //   VertexIter begin,end;
  //   boost::tie(begin,end) = vertices(g);
  //   for (VertexIter it=begin;it!=end;++it) {
  //     Vertex v = *it;
  //     if (in_degree(v, g) == 0) ret.insert(Vertex2Node[v]);
  //   }
  //   return ret;
  // }
  // template <typename T> std::set<T> Graph<T>::getExit() {
  //   std::set<T> ret;
  //   VertexIter begin,end;
  //   boost::tie(begin,end) = vertices(g);
  //   for (VertexIter it=begin;it!=end;++it) {
  //     Vertex v = *it;
  //     if (out_degree(v, g) == 0) ret.insert(Vertex2Node[v]);
  //   }
  //   return ret;
  // }
  

  
  template class hebigraph::Graph<CFGNode*>;
}
