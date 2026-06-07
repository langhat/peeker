#pragma once

#include <string>
#include <vector>
#include <map>

using std::string;
using std::vector;
using std::map;

class Model {
	map<string, map<string, float>> record;
	const size_t meaningMax = 25;
	const float relMin = 0.05;

	float next(float k) const noexcept {
		return k > 1 ? 1 / k : 1;
	}
public:
	void train(const vector<string> &words) noexcept {
		for(size_t index = 0; index < words.size(); index++) {
			for(size_t jndex = 0; jndex < words.size(); jndex++) {
				if(jndex == index) continue;
				if(record[words[index]].size() > meaningMax) continue;

				record[words[index]][words[jndex]] += next(record[words[index]][words[jndex]]);
			}
		}
	}
	map<string, float> get(const vector<string> &words) noexcept {
		map<string, float> weights;
		for(const auto &word: words) {
			if(record[word].size() > meaningMax) continue;
			for(const auto &[key, val]:record[word]) {
				weights[key] += val;
			}
		}

		for(auto &[key, val] : weights) {
			if(record[key].size() > meaningMax) val = 0.0;
		}

		// Softmax it
		float sum_exp = 0.0f;
		for (const auto &[key, val] : weights) sum_exp += std::exp(val);
		for (auto &[key, val] : weights) val = std::exp(val) / sum_exp;

		return weights;
	}
	void wash(){} // TODO
	void load(const string &){} // TODO
	void save(const string &){} // TODO
};