
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/r_c_shortest_paths.hpp>
using namespace boost;

#include <set>
#include <vector>
#include <iostream>
using namespace std;

namespace Plandipitous
{


struct VertProp
{
    VertProp() : min_cost(0), max_cost(0) {}
    VertProp(int min, int max) : min_cost(min), max_cost(max) {}
    int min_cost;
    int max_cost;
};

struct ArcProp
{
    ArcProp(int cost) : arc_cost(cost) {}
    int arc_cost;
};

typedef adjacency_list<
    vecS,vecS,directedS,VertProp,ArcProp
> GraphType;


// ResourceContainer model
struct ResourceContainer
{
    int time;
    int cost;

    ResourceContainer(int time_, int cost_) : time(time_), cost(cost_) {}

    ResourceContainer& operator=( const ResourceContainer& other )
    {
        if( this == &other )
            return *this;
        this->~ResourceContainer();
        new( this ) ResourceContainer( other );
        return *this;
    }
};

bool operator==( const ResourceContainer& a,
                 const ResourceContainer& b )
{
    // TODO:  Return true iff all the resources in container 1 equal container 2.
    return (a.time == b.time) && (a.cost == b.cost);
}

bool operator<( const ResourceContainer& a,
                const ResourceContainer& b )
{
    // TODO:  Impose a total ordering on resource containers.  That is,
    // if resource X is the same, check resource Y, and so forth.
    return a.time == b.time? a.cost < b.cost : a.time < b.time;
}

// ResourceExtensionFunction model
class ResourceExtensionFunction
{
public:
    inline bool operator()( const GraphType& g,
                            ResourceContainer& new_cont,
                            const ResourceContainer& old_cont,
                            graph_traits<GraphType>::edge_descriptor ed
    ) const
    {
        const ArcProp& arc_prop = get( edge_bundle, g )[ed];
        const VertProp& vert_prop = get( vertex_bundle, g )[target( ed, g )];

        // TODO:  new container resources = old container resources + action costs.
        // (I think costs have to be added rather than subtracted because the dominance
        // function is using the <= operator to look for lower resource consumptions.)

        new_cont.time = old_cont.time + 1;
        new_cont.cost = old_cont.cost + arc_prop.arc_cost;

        std::cout << "Time " << new_cont.time << " cost " << old_cont.cost << " -> " << new_cont.cost << std::endl;

        // TODO:  Return true iff the extension is feasible.
        return new_cont.cost <= vert_prop.max_cost && new_cont.cost >= vert_prop.min_cost;
    }
};

// DominanceFunction model
class DominanceFunction
{
public:
  inline bool operator()( const ResourceContainer& res_cont_1,
                          const ResourceContainer& res_cont_2 ) const
  {

    // TODO:  Return true if all the resources in container 1 are <= to container 2.
    // (Or == for nominal (non-numeric) properties.)
    bool dominated = res_cont_1.time <= res_cont_2.time && res_cont_1.cost == res_cont_2.cost ;

    std::cout << "time: " << res_cont_1.time << "/" << res_cont_2.time << " cost: " << res_cont_1.cost << "/" << res_cont_2.cost << std::endl;

    return dominated;

    // must be "<=" here!!!
    // must NOT be "<"!!!
    // this is not a contradiction to the documentation
    // the documentation says:
    // "A label $l_1$ dominates a label $l_2$ if and only if both are resident
    // at the same vertex, and if, for each resource, the resource consumption
    // of $l_1$ is less than or equal to the resource consumption of $l_2$,
    // and if there is at least one resource where $l_1$ has a lower resource
    // consumption than $l_2$."
    // one can think of a new label with a resource consumption equal to that
    // of an old label as being dominated by that old label, because the new
    // one will have a higher number and is created at a later point in time,
    // so one can implicitly use the number or the creation time as a resource
    // for tie-breaking
  }
};

void do_test_2()
{
    GraphType g;

    typedef GraphType::vertex_descriptor vd;

    vd start = add_vertex( VertProp(-20,20), g );
    vd node3 = add_vertex( VertProp(-20,20), g );
    vd node5 = add_vertex( VertProp(-20,20), g );
    vd goal =  add_vertex( VertProp(2,2), g );

    typedef GraphType::edge_descriptor ed;

    add_edge( start, node3, ArcProp(3), g );
    add_edge( start, node3, ArcProp(-3), g );
    add_edge( node3, start, ArcProp(0), g );

    add_edge( start, node5, ArcProp(5), g );
    add_edge( start, node5, ArcProp(-5), g );
    add_edge( node5, start, ArcProp(0), g );

    add_edge( start, goal, ArcProp(0), g );

  std::vector< std::vector<ed> > opt_solutions;

  std::vector<std::vector<ed> >   opt_solutions_spptw;
  std::vector<ResourceContainer> pareto_opt_rcs_spptw;

  r_c_shortest_paths
  ( g,
    get( vertex_index, g ),
    get( vertex_index, g ),
    start,
    goal,
    opt_solutions_spptw,
    pareto_opt_rcs_spptw,
    ResourceContainer( 0, 0 ),
    ResourceExtensionFunction(),
    DominanceFunction(),
    std::allocator
      <r_c_shortest_paths_label
        <GraphType, ResourceContainer> >(),
          default_r_c_shortest_paths_visitor() );

  std::cout << "Number of optimal solutions: ";
  std::cout << static_cast<int>( opt_solutions.size() ) << std::endl;
  for( int i = 0; i < static_cast<int>( opt_solutions.size() ); ++i )
  {
    std::cout << "The " << i << "th shortest path is: ";
    std::cout << std::endl;
    for( int j = static_cast<int>( opt_solutions_spptw[i].size() ) - 1;
         j >= 0;
         --j )
        std::cout << source( opt_solutions_spptw[i][j], g ) << std::endl;
    std::cout << "cost: " << pareto_opt_rcs_spptw[i].cost << std::endl;
    std::cout << "time: " << pareto_opt_rcs_spptw[i].time << std::endl;
  }

  // utility function check_r_c_path example
  std::cout << std::endl;
  bool b_is_a_path_at_all = false;
  bool b_feasible = false;
  bool b_correctly_extended = false;
  ResourceContainer actual_final_resource_levels( 0, 0 );
  ed ed_last_extended_arc;
  check_r_c_path( g,
                  opt_solutions_spptw[0],
                  ResourceContainer( 0, 0 ),
                  true,
                  pareto_opt_rcs_spptw[0],
                  actual_final_resource_levels,
                  ResourceExtensionFunction(),
                  b_is_a_path_at_all,
                  b_feasible,
                  b_correctly_extended,
                  ed_last_extended_arc );
  if( !b_is_a_path_at_all )
    std::cout << "Not a path." << std::endl;
  if( !b_feasible )
    std::cout << "Not a feasible path." << std::endl;
  if( !b_correctly_extended )
    std::cout << "Not correctly extended." << std::endl;
  if( b_is_a_path_at_all && b_feasible && b_correctly_extended )
  {
    std::cout << "Actual final resource levels:" << std::endl;
    std::cout << "Cost: " << actual_final_resource_levels.cost << std::endl;
    std::cout << "Time: " << actual_final_resource_levels.time << std::endl;
    std::cout << "OK." << std::endl;
  }

}


}
