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
    void merge(Graph gother);

    std::set<T> getEntry();
    std::set<T> getExit();
    /**
     * Connect with other graph.
     */
    void connect(Graph gother);
    /**
     * Connect not from this graph's exit, but from node "from"
     */
    void connect(Graph gother, T from);
    // TODO Labeling
    /**
     * visualize by exporting to graph
     */
    std::string visualize(std::string (*labelFunc)(T));
  private:
    std::map<T,Vertex> Node2Vertex;
    std::map<Vertex,T> Vertex2Node;
    adjacency_list<vecS, vecS, bidirectionalS> g;
  };

  template <typename T> std::string visualize(Graph<T> g);
}

#endif /* GRAPH_H */
