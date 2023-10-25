#include "serialization.h"

#include "fstream"

namespace serialization {

void Serialize(transport::Catalogue& tc, const renderer::MapRenderer& render, const transport::TransportRouter& router, std::ostream& out){
    proto_transport::TransportCatalogue proto_tc;

	SerializeStops(tc, proto_tc);
	SerializeStopDistances(tc, proto_tc);
	SerializeBuses(tc, proto_tc);
    SerializeRender(render, proto_tc);
    SerializeStopIds(router, proto_tc);
    SerializeRouterSettings(router, proto_tc);
    SerializeGraph(router, proto_tc);

	proto_tc.SerializeToOstream(&out);
}
    
std::tuple<transport::Catalogue, renderer::MapRenderer, transport::TransportRouter> Deserialize(std::istream& input) {
    proto_transport::TransportCatalogue proto_tc;
	proto_tc.ParseFromIstream(&input);

	transport::Catalogue tc;

	DeserializeStops(tc, proto_tc);
	DeserializeStopDistances(tc, proto_tc);
	DeserializeBuses(tc, proto_tc);
    renderer::RenderSettings render_settings;
    renderer::MapRenderer renderer = DeserializeRenderSettings(render_settings, proto_tc);
    transport::TransportRouter router;
    DeserializeRouter(router, proto_tc);
	return std::make_tuple< transport::Catalogue, renderer::MapRenderer, transport::TransportRouter> ( std::move(tc), std::move(renderer), std::move(router) );
}


void SerializeStops(const transport::Catalogue& tc, proto_transport::TransportCatalogue& proto_tc) {
    const auto all_stops = tc.GetSortedAllStops();
    for (const auto& stop : all_stops) {
        proto_transport::Stop proto_stop;
        proto_stop.set_name(stop.second->name);
        proto_stop.mutable_coordinates()->set_lat(stop.second->coordinates.lat);
        proto_stop.mutable_coordinates()->set_lng(stop.second->coordinates.lng);
        for (const auto& bus : stop.second->buses_by_stop) {
			proto_stop.add_buses_by_stop(bus);
		}
		*proto_tc.add_stops() = std::move(proto_stop);
    }
}
    
void SerializeStopDistances(const transport::Catalogue& tc, proto_transport::TransportCatalogue& proto_tc) {
    const auto stop_distances = tc.GetStopDistances();
    for (const auto& [from_to, dist] : stop_distances) {
        proto_transport::StopDistances proto_distances;
        proto_distances.set_from(from_to.first->name);
        proto_distances.set_to(from_to.second->name);
        proto_distances.set_distance(dist);
        
        *proto_tc.add_stop_distances() = std::move(proto_distances);
    }
}
    
void SerializeBuses(const transport::Catalogue& tc, proto_transport::TransportCatalogue& proto_tc) {
    const auto all_buses = tc.GetSortedAllBuses();
    for (const auto& bus : all_buses) {
        proto_transport::Bus proto_bus;
        proto_bus.set_number(bus.second->number);
        for (const auto* stop : bus.second->stops) {
			*proto_bus.mutable_stops()->Add() = stop->name;
		}
		proto_bus.set_is_circle(bus.second->is_circle);
		*proto_tc.add_buses() = std::move(proto_bus);
    }
}

void SerializeRender(const renderer::MapRenderer& render, proto_transport::TransportCatalogue& proto_tc) {
    const auto& render_settings = render.GetRenderSettings();
    proto_map::RenderSettings proto_render_settings;
    proto_render_settings.set_width(render_settings.width);
    proto_render_settings.set_height(render_settings.height);
    proto_render_settings.set_padding(render_settings.padding);
    proto_render_settings.set_stop_radius(render_settings.stop_radius);
    proto_render_settings.set_line_width(render_settings.line_width);
    proto_render_settings.set_bus_label_font_size(render_settings.bus_label_font_size);
    *proto_render_settings.mutable_bus_label_offset() = SerializePoint(render_settings.bus_label_offset);
    proto_render_settings.set_stop_label_font_size(render_settings.stop_label_font_size);
    *proto_render_settings.mutable_stop_label_offset() = SerializePoint(render_settings.stop_label_offset);
    *proto_render_settings.mutable_underlayer_color() = SerializeColor(render_settings.underlayer_color);
    proto_render_settings.set_underlayer_width(render_settings.underlayer_width);
    for (const auto& color : render_settings.color_palette) {
        *proto_render_settings.add_color_palette() = SerializeColor(color);
    }
    *proto_tc.mutable_render_settings() = std::move(proto_render_settings);
}

proto_svg::Point SerializePoint(const svg::Point& point) {
    proto_svg::Point proto_point;
    proto_point.set_x(point.x);
    proto_point.set_y(point.y);
    return proto_point;
}

proto_svg::Color SerializeColor(const svg::Color& color) {
    proto_svg::Color proto_color;
    switch (color.index()) {
    case 1: {
        proto_color.set_name(std::get<1>(color));
        break;
    }
    case 2: {
        *proto_color.mutable_rgb() = SerializeRgb(std::get<2>(color));
        break;
    }
    case 3: {
        *proto_color.mutable_rgba() = SerializeRgba(std::get<3>(color));
        break;
    }
    }
    return proto_color;
}

proto_svg::Rgb SerializeRgb(const svg::Rgb& rgb) {
    proto_svg::Rgb proto_rgb;
    proto_rgb.set_r(rgb.red);
    proto_rgb.set_g(rgb.green);
    proto_rgb.set_b(rgb.blue);

    return proto_rgb;
}

proto_svg::Rgba SerializeRgba(const svg::Rgba& rgba) {
    proto_svg::Rgba proto_rgba;
    *proto_rgba.mutable_rgb() = SerializeRgb(rgba);
    proto_rgba.set_opacity(rgba.opacity);

    return proto_rgba;
}

void SerializeRouterSettings(const transport::TransportRouter& router, proto_transport::TransportCatalogue& proto_tc) {
    transport::GetRouteData data;
    const auto& settings = data.GetRoutingSettings(router);
    proto_router::RoutingSettings proto_router_settings;
    proto_router_settings.set_wait_time(settings.bus_wait_time_);
    proto_router_settings.set_velocity(settings.bus_velocity_);
    *proto_tc.mutable_router()->mutable_router_settings() = std::move(proto_router_settings);
}

void SerializeGraph(const transport::TransportRouter& router, proto_transport::TransportCatalogue& proto_tc) {
    transport::GetRouteData data;
    const auto& graph = data.GetGraph(router);
    for (int i = 0; i < graph.GetEdgeCount(); ++i) {
        proto_graph::Edge proto_edge;
        const auto& edge = graph.GetEdge(i);
        proto_edge.set_name(edge.name);
        proto_edge.set_quality(edge.quality);
        proto_edge.set_from(edge.from);
        proto_edge.set_to(edge.to);
        proto_edge.set_weight(edge.weight);
        *proto_tc.mutable_router()->mutable_graph()->add_edges() = std::move(proto_edge);
    }
    for (int i = 0; i < graph.GetVertexCount(); ++i) {
        proto_graph::Vertex vertex;
        for (const auto& id : graph.GetIncidentEdges(i)) {
            vertex.add_edge_id(id);
        }
        *proto_tc.mutable_router()->mutable_graph()->add_vertexes() = vertex;
    }
}

void SerializeStopIds(const transport::TransportRouter& router, proto_transport::TransportCatalogue& proto_tc) {
    transport::GetRouteData data;
    const auto& stop_ids = data.GetStopIds(router);
    for (const auto& [name, id] : stop_ids) {
        proto_router::StopId proto_stop_id;
        proto_stop_id.set_name(name);
        proto_stop_id.set_id(id);
        *proto_tc.mutable_router()->add_stop_ids() = std::move(proto_stop_id);
    }
}

void DeserializeStops(transport::Catalogue& tc, const proto_transport::TransportCatalogue& proto_tc) {
    for (size_t i = 0; i < proto_tc.stops_size(); ++i) {
		const proto_transport::Stop& proto_stop = proto_tc.stops(i);
		tc.AddStop(proto_stop.name(), { proto_stop.coordinates().lat(), proto_stop.coordinates().lng() });
	}
}
    
void DeserializeStopDistances(transport::Catalogue& tc, const proto_transport::TransportCatalogue& proto_tc) {
    for (size_t i = 0; i < proto_tc.stop_distances_size(); ++i) {
		const proto_transport::StopDistances& proto_stop_distances = proto_tc.stop_distances(i);
		const transport::Stop* from = tc.FindStop(proto_stop_distances.from());
		const transport::Stop* to = tc.FindStop(proto_stop_distances.to());
		tc.SetDistance(from, to, proto_stop_distances.distance());
	}
}
    
void DeserializeBuses(transport::Catalogue& tc, const proto_transport::TransportCatalogue& proto_tc) {
    for (size_t i = 0; i < proto_tc.buses_size(); ++i) {
        const proto_transport::Bus& proto_bus = proto_tc.buses(i);
        std::vector<const transport::Stop*> stops(proto_bus.stops_size());
		for (size_t j = 0; j < stops.size(); ++j) {
			stops[j] = tc.FindStop(proto_bus.stops(j));
		}
        tc.AddRoute(proto_bus.number(), stops, proto_bus.is_circle());
    }
}

renderer::MapRenderer DeserializeRenderSettings(renderer::RenderSettings& render_settings, const proto_transport::TransportCatalogue& proto_tc) {
    const proto_map::RenderSettings& proto_render_settings = proto_tc.render_settings();
    render_settings.width = proto_render_settings.width();
    render_settings.height = proto_render_settings.height();
    render_settings.padding = proto_render_settings.padding();
    render_settings.stop_radius = proto_render_settings.stop_radius();
    render_settings.line_width = proto_render_settings.line_width();
    render_settings.bus_label_font_size = proto_render_settings.bus_label_font_size();
    render_settings.bus_label_offset = DeserializePoint(proto_render_settings.bus_label_offset());
    render_settings.stop_label_font_size = proto_render_settings.stop_label_font_size();
    render_settings.stop_label_offset = DeserializePoint(proto_render_settings.stop_label_offset());
    render_settings.underlayer_color = DeserializeColor(proto_render_settings.underlayer_color());
    render_settings.underlayer_width = proto_render_settings.underlayer_width();
    for (int i = 0; i < proto_render_settings.color_palette_size(); ++i) {
        render_settings.color_palette.push_back(DeserializeColor(proto_render_settings.color_palette(i)));
    }
    return render_settings;
}

svg::Point DeserializePoint(const proto_svg::Point& proto_point) {
    return { proto_point.x(), proto_point.y() };
}

svg::Color DeserializeColor(const proto_svg::Color& proto_color) {
    if (proto_color.has_rgb()) return svg::Rgb{ static_cast<uint8_t>(proto_color.rgb().r()),
                                                static_cast<uint8_t>(proto_color.rgb().g()),
                                                static_cast<uint8_t>(proto_color.rgb().b()) };
    else if (proto_color.has_rgba()) return svg::Rgba{ static_cast<uint8_t>(proto_color.rgba().rgb().r()),
                                                        static_cast<uint8_t>(proto_color.rgba().rgb().g()),
                                                        static_cast<uint8_t>(proto_color.rgba().rgb().b()),
                                                        proto_color.rgba().opacity() };
    else return proto_color.name();

    throw std::runtime_error("Error deserialized color");
}

void DeserializeRouter(transport::TransportRouter& router ,const proto_transport::TransportCatalogue& proto_tc) {
    transport::RoutingSettings settings = DeserializeRoutingSettings(proto_tc);
    StopById stop_ids = DeserializeStopById(proto_tc);
    Graph graph = DeserializeGraph(proto_tc);
    router.SetRoutingSettings(settings);
    router.SetStopByIds(stop_ids);
    router.SetGraph(graph);
}

transport::RoutingSettings DeserializeRoutingSettings(const proto_transport::TransportCatalogue& proto_tc) {
    int bus_wait_time = proto_tc.router().router_settings().wait_time();
    double velocity = proto_tc.router().router_settings().velocity();
    return { bus_wait_time, velocity };
}

StopById DeserializeStopById(const proto_transport::TransportCatalogue& proto_tc) {
    StopById stop_ids;
    for (const auto& proto_stop_id : proto_tc.router().stop_ids()) {
        stop_ids[proto_stop_id.name()] = proto_stop_id.id();
    }
    return stop_ids;
}

Graph DeserializeGraph(const proto_transport::TransportCatalogue& proto_tc) {
    const proto_graph::Graph& proto_graph = proto_tc.router().graph();
    graph::DirectedWeightedGraph<double> graph;
    std::vector<graph::Edge<double>> edges(proto_graph.edges_size());
    std::vector<std::vector<graph::EdgeId>> incidence_list(proto_graph.vertexes_size());
    for (int i = 0; i < edges.size(); ++i) {
        const proto_graph::Edge& proto_edge = proto_graph.edges(i);
        edges[i] = { proto_edge.name(),
                     static_cast<size_t>(proto_edge.quality()),
                     static_cast<size_t>(proto_edge.from()),
                     static_cast<size_t>(proto_edge.to()),
                     proto_edge.weight() };
        
    }
    for (size_t i = 0; i < incidence_list.size(); ++i) {
        const proto_graph::Vertex& proto_vertex = proto_graph.vertexes(i);
        incidence_list[i].reserve(proto_vertex.edge_id_size());
        for (const auto& id : proto_vertex.edge_id()) {
            incidence_list[i].push_back(id);
        }
    }
    graph.SetEdges(edges);
    graph.SetIncidence(incidence_list);
    return graph;
}

}