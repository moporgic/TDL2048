#pragma once
#ifndef MOPORGIC_NN_H_
#define MOPORGIC_NN_H_
/*
 * nn.h
 *
 *  Created on: 20170118
 *      Author: moporgic
 */
#include <algorithm>
#include <random>
#include <vector>
#include <cstdint>

namespace moporgic {

template<typename num_t = float>
struct neunet {
	typedef std::vector<num_t> input_t;
	typedef std::vector<num_t> output_t;
	typedef std::vector<std::vector<num_t>> weight_t;
	typedef std::vector<std::vector<num_t>> update_t;

	struct layer {
		output_t output;
		weight_t weight;
		update_t update;
		input_t delta;
		layer();
		layer(const size_t& node, const size_t& prev = 0, const uint64_t& seed = 0);
		void resize(const size_t& node, const size_t& prev = 0, const uint64_t& seed = 0);
		inline void copy_input(const input_t& input);
		inline void calculate_output(const input_t& input);
		inline void calculate_delta(const output_t& expect);
		inline void calculate_delta(const layer& next);
		inline void apply_update(const input_t& input, const num_t& eta, const num_t& momentum = 0);
	};

	typedef std::vector<layer> net_t;
	typedef std::vector<int> conf_t;

	net_t net;
	neunet();
	neunet(const conf_t& layers, const uint64_t& seed = 0);
	void resize(const conf_t& layers, const uint64_t& seed = 0);
	const output_t& forward(const input_t& input);
	void backward(const output_t& expect, const num_t& eta, const num_t& momentum = 0);
};

template<typename num_t>
neunet<num_t>::neunet() {}

template<typename num_t>
neunet<num_t>::neunet(const conf_t& layers, const uint64_t& seed) {
	resize(layers, seed);
}

template<typename num_t>
void neunet<num_t>::resize(const conf_t& layers, const uint64_t& seed) {
	std::default_random_engine engine(seed);
	std::uniform_int_distribution<uint64_t> dist;
	net.resize(layers.size());
	for (size_t i = 0; i < net.size(); i++) {
		net[i].resize(layers[i], i ? layers[i - 1] : 0, dist(engine));
	}
}

template<typename num_t>
const typename neunet<num_t>::output_t& neunet<num_t>::forward(const input_t& input) {
	net.front().copy_input(input);
	for (size_t i = 1; i < net.size(); i++) {
		net[i].calculate_output(net[i - 1].output);
	}
	return net.back().output;
}

template<typename num_t>
void neunet<num_t>::backward(const output_t& expect, const num_t& eta, const num_t& momentum) {
	net.back().calculate_delta(expect);
	for (size_t i = net.size() - 2; i; i--) {
		net[i].calculate_delta(net[i + 1]);
	}
	for (size_t i = net.size() - 1; i; i--) {
		net[i].apply_update(net[i - 1].output, eta, momentum);
	}
}

template<typename num_t>
neunet<num_t>::layer::layer() {}

template<typename num_t>
neunet<num_t>::layer::layer(const size_t& node, const size_t& prev, const uint64_t& seed) {
	resize(node, prev, seed);
}

template<typename num_t>
void neunet<num_t>::layer::resize(const size_t& node, const size_t& prev, const uint64_t& seed) {
	std::default_random_engine engine(seed);
	std::uniform_real_distribution<num_t> dist(-1, 1);
//	std::normal_distribution<num_t> dist(0.0, 1.0 / 3.0);
	weight.resize(node);
	update.resize(node);
	output.resize(node);
	delta.resize(node);
	for (size_t i = 0; i < node; i++) {
		weight[i].resize(prev);
		update[i].resize(prev);
		for (size_t k = 0; k < prev; k++) {
			weight[i][k] = dist(engine);
			update[i][k] = 0;
		}
	}
}

template<typename num_t> inline
void neunet<num_t>::layer::copy_input(const input_t& input) {
	std::copy(input.begin(), input.end(), output.begin());
}

template<typename num_t> inline
void neunet<num_t>::layer::calculate_output(const input_t& input) {
	for (size_t i = 0; i < weight.size(); i++) {
		num_t v = 0;
		for (size_t k = 0; k < input.size(); k++)
			v += weight[i][k] * input[k];
		output[i] = std::pow(1.0 + std::exp(-v), -1);
	}
}

template<typename num_t> inline
void neunet<num_t>::layer::calculate_delta(const output_t& expect) {
	for (size_t i = 0; i < weight.size(); i++) {
		num_t d = expect[i], o = output[i];
		delta[i] = (d - o) * o * (1 - o);
	}
}

template<typename num_t> inline
void neunet<num_t>::layer::calculate_delta(const layer& next) {
	for (size_t i = 0; i < weight.size(); i++) {
		num_t y = output[i], sum = 0;
		for (size_t k = 0; k < next.weight.size(); k++)
			sum += next.delta[k] * next.weight[k][i];
		delta[i] = y * (1 - y) * sum;
	}
}

template<typename num_t> inline
void neunet<num_t>::layer::apply_update(const input_t& input, const num_t& eta, const num_t& momentum) {
	for (size_t i = 0; i < weight.size(); i++) {
		num_t alpha = eta * delta[i];
		for (size_t k = 0; k < update[i].size(); k++) {
			update[i][k] = alpha * input[k] + momentum * update[i][k];
			weight[i][k] += update[i][k];
		}
	}
}

} // namespace moporgic

#endif /* MOPORGIC_NN_H_ */
