#pragma once

#include "router.h"
#include "transport_catalogue.h"

#include <memory>

namespace transport {

    struct RoutingSettings {
        int bus_wait_time_ = 0;
        double bus_velocity_ = 0.0;
    };

    class GetRouteData;

    class TransportRouter {
        friend class GetRouteData;
    public:
        using Graph = graph::DirectedWeightedGraph<double>;
        using Route = std::optional<std::vector<graph::Edge<double>>>;
        using StopById = std::map<std::string, graph::VertexId>;
        constexpr static double KMH_TO_MMIN = 100.0 / 6.0;
        
        TransportRouter() = default;

        TransportRouter(const RoutingSettings& settings, const Catalogue& catalogue) :
            settings_(settings), catalogue_(catalogue) {
            BuildGraph();
        }


        const Route FindRoute(const std::string_view stop_from, const std::string_view stop_to) const;
        void SetRoutingSettings(const RoutingSettings& settings);
        void SetGraph(Graph&& graph);
        void SetStopByIds(StopById stop_ids);
    private:
        void FillGraphByStop(const std::map<std::string_view, const Stop*>& stops, Graph& stops_graph);
        void FillGraphByBus(const std::map<std::string_view, const Bus*>& buses, Graph& stops_graph);
        void BuildGraph();

        RoutingSettings settings_;

        const Catalogue& catalogue_{};
        Graph graph_;
        StopById stop_ids_;
        std::unique_ptr<graph::Router<double>> router_; 
    };

    class GetRouteData  {
    public:
        const RoutingSettings& GetRoutingSettings(const transport::TransportRouter& router) const;
        const TransportRouter::StopById& GetStopIds(const transport::TransportRouter& router) const;
        const TransportRouter::Graph& GetGraph(const transport::TransportRouter& router) const;
    };
}