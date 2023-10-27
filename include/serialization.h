#pragma once

#include "json.h"
#include "transport_catalogue.pb.h"
#include "svg.pb.h"
#include "map_renderer.pb.h"
#include "transport_catalogue.h"
#include "graph.pb.h"
#include "transport_router.pb.h"
#include "request_handler.h"
#include "transport_router.h"

namespace serialization {

	using Graph = graph::DirectedWeightedGraph<double>;
	using Route = std::optional<std::vector<graph::Edge<double>>>;
	using StopById = std::map<std::string, graph::VertexId>;

	void Serialize(transport::Catalogue& tc, const renderer::MapRenderer& renderer, const transport::TransportRouter& router, std::ostream& out);
	proto_transport::TransportCatalogue ParseDB(std::istream& input);
	std::tuple<transport::Catalogue, renderer::MapRenderer> Deserialize(const proto_transport::TransportCatalogue& proto_tc);

	void SerializeStops(const transport::Catalogue& tc, proto_transport::TransportCatalogue& proto_tc);
	void SerializeStopDistances(const transport::Catalogue& tc, proto_transport::TransportCatalogue& proto_tc);
	void SerializeBuses(const transport::Catalogue& tc, proto_transport::TransportCatalogue& proto_tc);
	void SerializeRender(const renderer::MapRenderer& render, proto_transport::TransportCatalogue& proto_tc);
	proto_svg::Point SerializePoint(const svg::Point& point);
	proto_svg::Color SerializeColor(const svg::Color& color);
	proto_svg::Rgb SerializeRgb(const svg::Rgb& rgb);
	proto_svg::Rgba SerializeRgba(const svg::Rgba& rgba);
	void SerializeRouterSettings(const transport::TransportRouter& router, proto_transport::TransportCatalogue& proto_tc); \
	void SerializeGraph(const transport::TransportRouter& router, proto_transport::TransportCatalogue& proto_tc);
	void SerializeStopIds(const transport::TransportRouter& router, proto_transport::TransportCatalogue& proto_tc);

	void DeserializeStops(transport::Catalogue& tc, const proto_transport::TransportCatalogue& proto_tc);
	void DeserializeStopDistances(transport::Catalogue& tc, const proto_transport::TransportCatalogue& proto_tc);
	void DeserializeBuses(transport::Catalogue& tc, const proto_transport::TransportCatalogue& proto_tc);
	renderer::MapRenderer DeserializeRenderSettings(renderer::RenderSettings& render_settings, const proto_transport::TransportCatalogue& proto_tc);
	svg::Point DeserializePoint(const proto_svg::Point& proto_point);
	svg::Color DeserializeColor(const proto_svg::Color& proto_color);
	transport::TransportRouter DeserializeRouter(const proto_transport::TransportCatalogue& proto_tc);
	transport::RoutingSettings DeserializeRoutingSettings(const proto_transport::TransportCatalogue& proto_tc);
	StopById DeserializeStopById(const proto_transport::TransportCatalogue& proto_tc);
	Graph DeserializeGraph(const proto_transport::TransportCatalogue& proto_tc);

} // serialization