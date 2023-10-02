#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

#include <sstream>
#include <optional>

class RequestHandler {
public:
    using Route = std::optional<std::vector<graph::Edge<double>>>;
    using Graph = graph::DirectedWeightedGraph<double>;
    RequestHandler(const transport::Catalogue& catalogue, const renderer::MapRenderer& renderer,const transport::TransportRouter& router)
        : catalogue_(catalogue)
        , renderer_(renderer)
        , transport_router_(router)
    {
    }
    
    
    std::optional<transport::BusStat> GetBusStat(const std::string_view bus_number) const;
    const std::set<std::string> GetBusesByStop(std::string_view stop_name) const;
    bool IsBusNumber(const std::string_view bus_number) const;
    bool IsStopName(const std::string_view stop_name) const;
    const Route GetOptimalRoute(const std::string_view stop_from, const std::string_view stop_to) const;
    svg::Document RenderMap() const;

private:
    const transport::Catalogue& catalogue_;
    const renderer::MapRenderer& renderer_;
    const transport::TransportRouter& transport_router_;
};