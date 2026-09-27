/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * (c) ZeroTier, Inc.
 * https://www.zerotier.com/
 */

#ifndef METRICS_H_
#define METRICS_H_

#if defined(__SWITCH__)
/* ZT_METRICS_NULL: the console has no use for Prometheus counters, and the registry they live in costs about
 * 110 objects built during static initialization plus a file-writer thread. Every metric is an empty type
 * that swallows the calls the node makes; nothing is registered, nothing is allocated. */
#include <cstdint>
#include <vector>
#include <initializer_list>
#include <thread>
#include <chrono>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
namespace ZeroTier {
namespace Metrics {
/* A stub that is touched says so, once, through zt_stub_hit() (weak: the embedding program defines it, and
 * without one the stub is silent). Once per metric, not per call: these are bumped on every packet. */
extern "C" void zt_stub_hit(const char* what) __attribute__((weak));
struct Null {
	const char* name;
	bool hit;
	constexpr Null(const char* n = "metric") : name(n), hit(false) {}
	void touch() {
		if (! hit) {
			hit = true;
			if (zt_stub_hit)
				zt_stub_hit(name);
		}
	}
	template <typename T> Null& operator=(const T&) { touch(); return *this; }
	Null(const Null&) = default;
	Null& operator=(const Null&) { return *this; }
	Null& operator++() { touch(); return *this; }
	Null operator++(int) { touch(); return *this; }
	Null& operator--() { touch(); return *this; }
	Null operator--(int) { touch(); return *this; }
	template <typename T> Null& operator+=(const T&) { touch(); return *this; }
	template <typename T> Null& operator-=(const T&) { touch(); return *this; }
	template <typename... A> void Increment(A&&...) { touch(); }
	template <typename... A> void Decrement(A&&...) { touch(); }
	template <typename... A> void Set(A&&...) { touch(); }
	template <typename... A> void Observe(A&&...) { touch(); }
	template <typename... A> Null& Add(A&&...) { touch(); return *this; }
	template <typename... A> Null& Add(std::initializer_list<std::pair<const std::string, std::string> >, A&&...) { touch(); return *this; }
};
typedef Null counter_t;
typedef Null gauge_t;
typedef Null& histogram_ref_t;
inline Null packets{"packets"};
inline Null pkt_nop_in{"pkt_nop_in"};
inline Null pkt_error_in{"pkt_error_in"};
inline Null pkt_ack_in{"pkt_ack_in"};
inline Null pkt_qos_in{"pkt_qos_in"};
inline Null pkt_hello_in{"pkt_hello_in"};
inline Null pkt_ok_in{"pkt_ok_in"};
inline Null pkt_whois_in{"pkt_whois_in"};
inline Null pkt_rendezvous_in{"pkt_rendezvous_in"};
inline Null pkt_frame_in{"pkt_frame_in"};
inline Null pkt_ext_frame_in{"pkt_ext_frame_in"};
inline Null pkt_echo_in{"pkt_echo_in"};
inline Null pkt_multicast_like_in{"pkt_multicast_like_in"};
inline Null pkt_network_credentials_in{"pkt_network_credentials_in"};
inline Null pkt_network_config_request_in{"pkt_network_config_request_in"};
inline Null pkt_network_config_in{"pkt_network_config_in"};
inline Null pkt_multicast_gather_in{"pkt_multicast_gather_in"};
inline Null pkt_multicast_frame_in{"pkt_multicast_frame_in"};
inline Null pkt_push_direct_paths_in{"pkt_push_direct_paths_in"};
inline Null pkt_user_message_in{"pkt_user_message_in"};
inline Null pkt_remote_trace_in{"pkt_remote_trace_in"};
inline Null pkt_path_negotiation_request_in{"pkt_path_negotiation_request_in"};
inline Null pkt_nop_out{"pkt_nop_out"};
inline Null pkt_error_out{"pkt_error_out"};
inline Null pkt_ack_out{"pkt_ack_out"};
inline Null pkt_qos_out{"pkt_qos_out"};
inline Null pkt_hello_out{"pkt_hello_out"};
inline Null pkt_ok_out{"pkt_ok_out"};
inline Null pkt_whois_out{"pkt_whois_out"};
inline Null pkt_rendezvous_out{"pkt_rendezvous_out"};
inline Null pkt_frame_out{"pkt_frame_out"};
inline Null pkt_ext_frame_out{"pkt_ext_frame_out"};
inline Null pkt_echo_out{"pkt_echo_out"};
inline Null pkt_multicast_like_out{"pkt_multicast_like_out"};
inline Null pkt_network_credentials_out{"pkt_network_credentials_out"};
inline Null pkt_network_config_request_out{"pkt_network_config_request_out"};
inline Null pkt_network_config_out{"pkt_network_config_out"};
inline Null pkt_multicast_gather_out{"pkt_multicast_gather_out"};
inline Null pkt_multicast_frame_out{"pkt_multicast_frame_out"};
inline Null pkt_push_direct_paths_out{"pkt_push_direct_paths_out"};
inline Null pkt_user_message_out{"pkt_user_message_out"};
inline Null pkt_remote_trace_out{"pkt_remote_trace_out"};
inline Null pkt_path_negotiation_request_out{"pkt_path_negotiation_request_out"};
inline Null packet_errors{"packet_errors"};
inline Null pkt_error_obj_not_found_in{"pkt_error_obj_not_found_in"};
inline Null pkt_error_unsupported_op_in{"pkt_error_unsupported_op_in"};
inline Null pkt_error_identity_collision_in{"pkt_error_identity_collision_in"};
inline Null pkt_error_need_membership_cert_in{"pkt_error_need_membership_cert_in"};
inline Null pkt_error_network_access_denied_in{"pkt_error_network_access_denied_in"};
inline Null pkt_error_unwanted_multicast_in{"pkt_error_unwanted_multicast_in"};
inline Null pkt_error_authentication_required_in{"pkt_error_authentication_required_in"};
inline Null pkt_error_internal_server_error_in{"pkt_error_internal_server_error_in"};
inline Null pkt_error_obj_not_found_out{"pkt_error_obj_not_found_out"};
inline Null pkt_error_unsupported_op_out{"pkt_error_unsupported_op_out"};
inline Null pkt_error_identity_collision_out{"pkt_error_identity_collision_out"};
inline Null pkt_error_need_membership_cert_out{"pkt_error_need_membership_cert_out"};
inline Null pkt_error_network_access_denied_out{"pkt_error_network_access_denied_out"};
inline Null pkt_error_unwanted_multicast_out{"pkt_error_unwanted_multicast_out"};
inline Null pkt_error_authentication_required_out{"pkt_error_authentication_required_out"};
inline Null pkt_error_internal_server_error_out{"pkt_error_internal_server_error_out"};
inline Null data{"data"};
inline Null udp_send{"udp_send"};
inline Null udp_recv{"udp_recv"};
inline Null tcp_send{"tcp_send"};
inline Null tcp_recv{"tcp_recv"};
inline Null network_num_joined{"network_num_joined"};
inline Null network_num_multicast_groups{"network_num_multicast_groups"};
inline Null network_packets{"network_packets"};
inline Null peer_latency{"peer_latency"};
inline Null peer_path_count{"peer_path_count"};
inline Null peer_packets{"peer_packets"};
inline Null peer_packet_errors{"peer_packet_errors"};
inline Null network_count{"network_count"};
inline Null member_count{"member_count"};
inline Null network_changes{"network_changes"};
inline Null member_changes{"member_changes"};
inline Null member_auths{"member_auths"};
inline Null member_deauths{"member_deauths"};
inline Null network_config_request_queue_size{"network_config_request_queue_size"};
inline Null sso_expiration_checks{"sso_expiration_checks"};
inline Null sso_member_deauth{"sso_member_deauth"};
inline Null network_config_request{"network_config_request"};
inline Null network_config_request_threads{"network_config_request_threads"};
inline Null db_get_network{"db_get_network"};
inline Null db_get_network_and_member{"db_get_network_and_member"};
inline Null db_get_network_and_member_and_summary{"db_get_network_and_member_and_summary"};
inline Null db_get_member_list{"db_get_member_list"};
inline Null db_get_network_list{"db_get_network_list"};
inline Null db_member_change{"db_member_change"};
inline Null db_network_change{"db_network_change"};
inline Null pgsql_mem_notification{"pgsql_mem_notification"};
inline Null pgsql_net_notification{"pgsql_net_notification"};
inline Null pgsql_node_checkin{"pgsql_node_checkin"};
inline Null pgsql_commit_ticks{"pgsql_commit_ticks"};
inline Null db_get_sso_info{"db_get_sso_info"};
inline Null redis_mem_notification{"redis_mem_notification"};
inline Null redis_net_notification{"redis_net_notification"};
inline Null redis_node_checkin{"redis_node_checkin"};
inline Null conn_counter{"conn_counter"};
inline Null max_pool_size{"max_pool_size"};
inline Null min_pool_size{"min_pool_size"};
inline Null pool_avail{"pool_avail"};
inline Null pool_in_use{"pool_in_use"};
inline Null pool_errors{"pool_errors"};
}	// namespace Metrics
}	// namespace ZeroTier
#else

// clang-format off
#include <prometheus/simpleapi.h>
#include <prometheus/histogram.h>
// clang-format on

namespace prometheus {
namespace simpleapi {
extern std::shared_ptr<Registry> registry_ptr;
}
}	// namespace prometheus

namespace ZeroTier {
namespace Metrics {
// Packet Type Counts
extern prometheus::simpleapi::counter_family_t packets;

// incoming packets
extern prometheus::simpleapi::counter_metric_t pkt_nop_in;
extern prometheus::simpleapi::counter_metric_t pkt_error_in;
extern prometheus::simpleapi::counter_metric_t pkt_ack_in;
extern prometheus::simpleapi::counter_metric_t pkt_qos_in;
extern prometheus::simpleapi::counter_metric_t pkt_hello_in;
extern prometheus::simpleapi::counter_metric_t pkt_ok_in;
extern prometheus::simpleapi::counter_metric_t pkt_whois_in;
extern prometheus::simpleapi::counter_metric_t pkt_rendezvous_in;
extern prometheus::simpleapi::counter_metric_t pkt_frame_in;
extern prometheus::simpleapi::counter_metric_t pkt_ext_frame_in;
extern prometheus::simpleapi::counter_metric_t pkt_echo_in;
extern prometheus::simpleapi::counter_metric_t pkt_multicast_like_in;
extern prometheus::simpleapi::counter_metric_t pkt_network_credentials_in;
extern prometheus::simpleapi::counter_metric_t pkt_network_config_request_in;
extern prometheus::simpleapi::counter_metric_t pkt_network_config_in;
extern prometheus::simpleapi::counter_metric_t pkt_multicast_gather_in;
extern prometheus::simpleapi::counter_metric_t pkt_multicast_frame_in;
extern prometheus::simpleapi::counter_metric_t pkt_push_direct_paths_in;
extern prometheus::simpleapi::counter_metric_t pkt_user_message_in;
extern prometheus::simpleapi::counter_metric_t pkt_remote_trace_in;
extern prometheus::simpleapi::counter_metric_t pkt_path_negotiation_request_in;

// outgoing packets
extern prometheus::simpleapi::counter_metric_t pkt_nop_out;
extern prometheus::simpleapi::counter_metric_t pkt_error_out;
extern prometheus::simpleapi::counter_metric_t pkt_ack_out;
extern prometheus::simpleapi::counter_metric_t pkt_qos_out;
extern prometheus::simpleapi::counter_metric_t pkt_hello_out;
extern prometheus::simpleapi::counter_metric_t pkt_ok_out;
extern prometheus::simpleapi::counter_metric_t pkt_whois_out;
extern prometheus::simpleapi::counter_metric_t pkt_rendezvous_out;
extern prometheus::simpleapi::counter_metric_t pkt_frame_out;
extern prometheus::simpleapi::counter_metric_t pkt_ext_frame_out;
extern prometheus::simpleapi::counter_metric_t pkt_echo_out;
extern prometheus::simpleapi::counter_metric_t pkt_multicast_like_out;
extern prometheus::simpleapi::counter_metric_t pkt_network_credentials_out;
extern prometheus::simpleapi::counter_metric_t pkt_network_config_request_out;
extern prometheus::simpleapi::counter_metric_t pkt_network_config_out;
extern prometheus::simpleapi::counter_metric_t pkt_multicast_gather_out;
extern prometheus::simpleapi::counter_metric_t pkt_multicast_frame_out;
extern prometheus::simpleapi::counter_metric_t pkt_push_direct_paths_out;
extern prometheus::simpleapi::counter_metric_t pkt_user_message_out;
extern prometheus::simpleapi::counter_metric_t pkt_remote_trace_out;
extern prometheus::simpleapi::counter_metric_t pkt_path_negotiation_request_out;

// Packet Error Counts
extern prometheus::simpleapi::counter_family_t packet_errors;

// incoming errors
extern prometheus::simpleapi::counter_metric_t pkt_error_obj_not_found_in;
extern prometheus::simpleapi::counter_metric_t pkt_error_unsupported_op_in;
extern prometheus::simpleapi::counter_metric_t pkt_error_identity_collision_in;
extern prometheus::simpleapi::counter_metric_t pkt_error_need_membership_cert_in;
extern prometheus::simpleapi::counter_metric_t pkt_error_network_access_denied_in;
extern prometheus::simpleapi::counter_metric_t pkt_error_unwanted_multicast_in;
extern prometheus::simpleapi::counter_metric_t pkt_error_authentication_required_in;
extern prometheus::simpleapi::counter_metric_t pkt_error_internal_server_error_in;

// outgoing errors
extern prometheus::simpleapi::counter_metric_t pkt_error_obj_not_found_out;
extern prometheus::simpleapi::counter_metric_t pkt_error_unsupported_op_out;
extern prometheus::simpleapi::counter_metric_t pkt_error_identity_collision_out;
extern prometheus::simpleapi::counter_metric_t pkt_error_need_membership_cert_out;
extern prometheus::simpleapi::counter_metric_t pkt_error_network_access_denied_out;
extern prometheus::simpleapi::counter_metric_t pkt_error_unwanted_multicast_out;
extern prometheus::simpleapi::counter_metric_t pkt_error_authentication_required_out;
extern prometheus::simpleapi::counter_metric_t pkt_error_internal_server_error_out;

// Data Sent/Received Metrics
extern prometheus::simpleapi::counter_family_t data;
extern prometheus::simpleapi::counter_metric_t udp_send;
extern prometheus::simpleapi::counter_metric_t udp_recv;
extern prometheus::simpleapi::counter_metric_t tcp_send;
extern prometheus::simpleapi::counter_metric_t tcp_recv;

// Network Metrics
extern prometheus::simpleapi::gauge_metric_t network_num_joined;
extern prometheus::simpleapi::gauge_family_t network_num_multicast_groups;
extern prometheus::simpleapi::counter_family_t network_packets;

#ifndef ZT_NO_PEER_METRICS
// Peer Metrics
extern prometheus::CustomFamily<prometheus::Histogram<uint64_t> >& peer_latency;
extern prometheus::simpleapi::gauge_family_t peer_path_count;
extern prometheus::simpleapi::counter_family_t peer_packets;
extern prometheus::simpleapi::counter_family_t peer_packet_errors;
#endif

// General Controller Metrics
extern prometheus::simpleapi::gauge_metric_t network_count;
extern prometheus::simpleapi::gauge_metric_t member_count;
extern prometheus::simpleapi::counter_metric_t network_changes;
extern prometheus::simpleapi::counter_metric_t member_changes;
extern prometheus::simpleapi::counter_metric_t member_auths;
extern prometheus::simpleapi::counter_metric_t member_deauths;

extern prometheus::simpleapi::gauge_metric_t network_config_request_queue_size;
extern prometheus::simpleapi::counter_metric_t sso_expiration_checks;
extern prometheus::simpleapi::counter_metric_t sso_member_deauth;
extern prometheus::simpleapi::counter_metric_t network_config_request;
extern prometheus::simpleapi::gauge_metric_t network_config_request_threads;

extern prometheus::simpleapi::counter_metric_t db_get_network;
extern prometheus::simpleapi::counter_metric_t db_get_network_and_member;
extern prometheus::simpleapi::counter_metric_t db_get_network_and_member_and_summary;
extern prometheus::simpleapi::counter_metric_t db_get_member_list;
extern prometheus::simpleapi::counter_metric_t db_get_network_list;
extern prometheus::simpleapi::counter_metric_t db_member_change;
extern prometheus::simpleapi::counter_metric_t db_network_change;

#ifdef ZT_CONTROLLER_USE_LIBPQ
// Central Controller Metrics
extern prometheus::simpleapi::counter_metric_t pgsql_mem_notification;
extern prometheus::simpleapi::counter_metric_t pgsql_net_notification;
extern prometheus::simpleapi::counter_metric_t pgsql_node_checkin;
extern prometheus::simpleapi::counter_metric_t pgsql_commit_ticks;
extern prometheus::simpleapi::counter_metric_t db_get_sso_info;

extern prometheus::simpleapi::counter_metric_t redis_mem_notification;
extern prometheus::simpleapi::counter_metric_t redis_net_notification;
extern prometheus::simpleapi::counter_metric_t redis_node_checkin;

// Central DB Pool Metrics
extern prometheus::simpleapi::counter_metric_t conn_counter;
extern prometheus::simpleapi::counter_metric_t max_pool_size;
extern prometheus::simpleapi::counter_metric_t min_pool_size;
extern prometheus::simpleapi::gauge_metric_t pool_avail;
extern prometheus::simpleapi::gauge_metric_t pool_in_use;
extern prometheus::simpleapi::counter_metric_t pool_errors;
#endif
typedef prometheus::simpleapi::counter_metric_t counter_t;
typedef prometheus::simpleapi::gauge_metric_t gauge_t;
typedef prometheus::Histogram<uint64_t>& histogram_ref_t;
}	// namespace Metrics
}	// namespace ZeroTier

#endif	 // !__SWITCH__

#endif	 // METRICS_H_
