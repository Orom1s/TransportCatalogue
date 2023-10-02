#include "request_handler.h"

std::optional<transport::BusStat> RequestHandler::GetBusStat(const std::string_view bus_number) const {
    return catalogue_.GetBusStat(bus_number);
}

const std::set<std::string> RequestHandler::GetBusesByStop(std::string_view stop_name) const {
    return catalogue_.FindStop(stop_name)->buses_by_stop;
}

bool RequestHandler::IsBusNumber(const std::string_view bus_number) const {
    return catalogue_.FindRoute(bus_number);
}

bool RequestHandler::IsStopName(const std::string_view stop_name) const {
    return catalogue_.FindStop(stop_name);
}

const RequestHandler::Route RequestHandler::GetOptimalRoute(const std::string_view stop_from, const std::string_view stop_to) const {
    return transport_router_.FindRoute(stop_from, stop_to);
}

svg::Document RequestHandler::RenderMap() const {
    return renderer_.GetSVG(catalogue_.GetSortedAllBuses());
}