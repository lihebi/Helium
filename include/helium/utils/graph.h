#ifndef GRAPH_H
#define GRAPH_H

#include <boost/utility.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/visitors.hpp>

// using namespace boost;


namespace hebigraph {

  using namespace boost;
// using namespace std;
  // typedef pair<int,int> Edge;

  typedef adjacency_list<vecS, vecS, bidirectionalS> GraphType;
  
  typedef graph_traits<GraphType>::vertex_descriptor Vertex;
  typedef graph_traits<GraphType>::edge_descriptor Edge;
  typedef graph_traits<GraphType>::vertex_iterator VertexIter;
  typedef graph_traits<GraphType>::edge_iterator EdgeIter;


  struct cycle_detector : public dfs_visitor<>
  {
    cycle_detector(bool& has_cycle) 
      : m_has_cycle(has_cycle) { }

    template <class Edge, class Graph>
    void back_edge(Edge, Graph&) { m_has_cycle = true; }
  protected:
    bool& m_has_cycle;
  };

  
  template <typename T> class Graph {
  public:
    Graph() {}
    ~Graph() {}
    
    bool hasEdge(T x, T y) {
      return edge(Node2Vertex[x], Node2Vertex[y], g).second;
    }
    void addEdge(T x, T y) {
      if (hasEdge(x, y)) return;
      add_edge(Node2Vertex[x], Node2Vertex[y], g);
    }
    void removeEdge(T x, T y) {
      if (!hasEdge(x,y)) return;
      remove_edge(Node2Vertex[x], Node2Vertex[y], g);
    }

    bool hasNode(T x) {
      return Node2Vertex.count(x) == 1;
    }
    void addNode(T x) {
      if (!hasNode(x)) {
        Vertex vertex = add_vertex(g);
        Node2Vertex[x] = vertex;
        Vertex2Node[vertex] = x;
      }
    }
    void removeNode(T x) {
      if (Node2Vertex.count(x) == 1) {
        remove_vertex(Node2Vertex[x], g);
        Vertex2Node.erase(Node2Vertex[x]);
        Node2Vertex.erase(x);
      }
    }

    void dump(std::ostream &os) {
      os << "Num of Vertices: " << num_vertices(g) << "\n";
      os << "Num of Edges: " << num_edges(g) << "\n";
      VertexIter begin,end;
      boost::tie(begin, end) = vertices(g);
      os << "Vertices:\n";
      for (VertexIter it=begin;it!=end;it++) {
        Vertex v = *it;
        os << v << ": ";
        // Vertex2Node[v]->dump(os);
        os << "\n";
      }
      os << "\n";
    }

    bool hasCycle() {
      bool has_cycle = false;
      cycle_detector vis(has_cycle);
      depth_first_search(g, visitor(vis));
      // cout << "The graph has a cycle? " << has_cycle << endl;
      return has_cycle;
    }

    std::vector<T> topoSort() {
      typedef std::list<Vertex> MakeOrder;
      MakeOrder::iterator i;
      MakeOrder make_order;

      topological_sort(g, std::front_inserter(make_order));
      // cout << "make ordering: ";
      // for (i = make_order.begin();
      //      i != make_order.end(); ++i) 
      //   cout << name[*i] << " ";
      
      std::vector<T> ret;
      for (Vertex v : make_order) {
        ret.push_back(Vertex2Node[v]);
      }
      return ret;
    }

    /**
     * Merge one graph into this graph
     * Merging all the nodes and edges
     */
    void merge(Graph gother) {
      // get the nodes
      // create nodes
      {
        VertexIter begin,end;
        boost::tie(begin,end) = vertices(gother);
        for (VertexIter it=begin;it!=end;++it) {
          Vertex v = *it;
          addNode(gother.Vertex2Node[v]);
        }
      }
      {
        // get edges
        // create edges
        EdgeIter begin,end;
        boost::tie(begin,end) = edges(gother);
        for (EdgeIter it=begin;it!=end;++it) {
          Edge e = *it;
          Vertex src = source(e, gother);
          Vertex dst = target(e, gother);
          addEdge(gother.Vertex2Node[src], gother.Vertex2Node[dst]);
        }
      }
    }

    std::set<T> getEntry() {
      std::set<T> ret;
      VertexIter begin,end;
      boost::tie(begin,end) = vertices(g);
      for (VertexIter it=begin;it!=end;++it) {
        Vertex v = *it;
        if (in_degree(v, g) == 0) ret.insert(Vertex2Node[v]);
      }
      return ret;
    }
    std::set<T> getExit() {
      std::set<T> ret;
      VertexIter begin,end;
      boost::tie(begin,end) = vertices(g);
      for (VertexIter it=begin;it!=end;++it) {
        Vertex v = *it;
        if (out_degree(v, g) == 0) ret.insert(Vertex2Node[v]);
      }
      return ret;
    }
    /**
     * Connect with other graph.
     */
    void connect(Graph gother) {
      // 1. record exit nodes and other's entry
      std::set<T> orig_exit = getExit();
      std::set<T> other_entry = gother.getEntry();
      // 2. merge gother
      merge(gother);
      // 3. Add edge from this exit to gother's entry
      for (T x : orig_exit) {
        for (T y : other_entry) {
          addEdge(x, y);
        }
      }
    }
    /**
     * Connect not from this graph's exit, but from node "from"
     */
    void connect(Graph gother, T from) {
      std::set<T> other_entry = gother.getEntry();
      merge(gother);
      for (T to : other_entry) {
        addEdge(from, to);
      }
    }
    // TODO Labeling
    // TODO export to dot, agg, etc

  private:
    std::map<T,Vertex> Node2Vertex;
    std::map<Vertex,T> Vertex2Node;
    adjacency_list<vecS, vecS, bidirectionalS> g;
  };
}

#endif /* GRAPH_H */
