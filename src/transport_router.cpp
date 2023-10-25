#include "transport_router.h"

namespace transport {

    const TransportRouter::Route TransportRouter::FindRoute(const std::string_view stop_from, const std::string_view stop_to) const {
        const auto& routing = router_->BuildRoute(stop_ids_.at(std::string(stop_from)), stop_ids_.at(std::string(stop_to)));
        if (!routing) {
            return std::nullopt;
        }
        Route route_by_id(std::nullopt);
        for (const auto id : routing.value().edges) {
            route_by_id->push_back(graph_.GetEdge(id));
        }
        return route_by_id;
    }

    void TransportRouter::SetRoutingSettings(const RoutingSettings& settings) {
        settings_ = settings;
    }

    void TransportRouter::SetGraph(Graph graph) {
        graph_ = std::move(graph);
        router_ = std::make_unique<graph::Router<double>>(graph_);
    }

    void TransportRouter::SetStopByIds(StopById stop_ids) {
        stop_ids_ = std::move(stop_ids);
    }


    void TransportRouter::FillGraphByStop(const std::map<std::string_view, const Stop*>& stops, Graph& stops_graph) {
        std::map<std::string, graph::VertexId> stop_ids;
        graph::VertexId vertex_id = 0;

        for (const auto& [stop_name, stop_info] : stops) {
            stop_ids[stop_info->name] = vertex_id;
            stops_graph.AddEdge({
                    stop_info->name,
                    0,
                    vertex_id,
                    ++vertex_id,
                    static_cast<double>(settings_.bus_wait_time_)
                });
            ++vertex_id;
        }
        stop_ids_ = std::move(stop_ids);
    }

    void TransportRouter::FillGraphByBus(const std::map<std::string_view, const Bus*>& buses, Graph& stops_graph) {
        for_each(
            buses.begin(),
            buses.end(),
            [&stops_graph, this](const auto& item) {
                const auto& bus_info = item.second;
                const auto& stops = bus_info->stops;
                size_t stops_count = stops.size();
                for (size_t i = 0; i < stops_count; ++i) {
                    for (size_t j = i + 1; j < stops_count; ++j) {
                        const Stop* stop_from = stops[i];
                        const Stop* stop_to = stops[j];
                        int dist_sum = 0;
                        int dist_sum_inverse = 0;
                        for (size_t k = i + 1; k <= j; ++k) {
                            dist_sum += catalogue_.GetDistance(stops[k - 1], stops[k]);
                            dist_sum_inverse += catalogue_.GetDistance(stops[k], stops[k - 1]);
                        }
                        stops_graph.AddEdge({ bus_info->number,
                                              j - i,
                                              stop_ids_.at(stop_from->name) + 1,
                                              stop_ids_.at(stop_to->name),
                                              static_cast<double>(dist_sum) / (settings_.bus_velocity_ * KMH_TO_MMIN) });

                        if (!bus_info->is_circle) {
                            stops_graph.AddEdge({ bus_info->number,
                                                  j - i,
                                                  stop_ids_.at(stop_to->name) + 1,
                                                  stop_ids_.at(stop_from->name),
                                                  static_cast<double>(dist_sum_inverse) / (settings_.bus_velocity_ * KMH_TO_MMIN) });
                        }
                    }
                }
            });
    }

    void TransportRouter::BuildGraph() {
        const auto& all_stops = catalogue_.GetSortedAllStops();
        const auto& all_buses = catalogue_.GetSortedAllBuses();
        Graph stops_graph(all_stops.size() * 2);
        FillGraphByStop(all_stops, stops_graph);
        FillGraphByBus(all_buses, stops_graph);
        graph_ = std::move(stops_graph);
        router_ = std::make_unique<graph::Router<double>>(graph_);
    }
    
    const RoutingSettings& GetRouteData::GetRoutingSettings(const transport::TransportRouter& router) const {
        return router.settings_;
    }

    const TransportRouter::StopById& GetRouteData::GetStopIds(const transport::TransportRouter& router) const
    {
        return router.stop_ids_;
    }

    const TransportRouter::Graph& GetRouteData::GetGraph(const transport::TransportRouter& router) const
    {
        return router.graph_;
    }

}