#include <iostream>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <sstream>
#include "model.cpp"
#include "split.cpp"

using namespace std;

Model md;
const string datas = "yuki is cute.yuki is top 1.safety is always first.";

bool espInput(const string &input) {
    if(input == "quit") {
        exit(0);
    }
    if(input.find("train ", 0) == 0) {
        string filename = input.substr(6);
        ifstream file(filename);
        if(!file.is_open()) {
            cerr << "Can not open file: " << filename << endl;
            return true;
        }

        stringstream buffer;
        buffer << file.rdbuf();
        string content = buffer.str();
        file.close();

        auto sentences = split2(content);
        for (const auto& words : sentences) {
            md.train(words);
        }
        cout << "Train done. Load " << sentences.size() << " sentences." << endl;
        return true;
    }
    return false;
}

int main() {
	for(const auto words: split2(datas)) {
		md.train(words);
	}
	string input;
	while(1) {
		cout << "\n? " << flush;
		getline(cin, input);
		if (espInput(input)) continue;
		auto ques = split(input);
		auto probs = md.get(ques);

		vector<pair<string, float>> sorted(probs.begin(), probs.end());
		sort(sorted.begin(), sorted.end(),
		     [](const auto& a, const auto& b) { return a.second > b.second; });

		for (int i = 0; i < min(5, (int)sorted.size()); ++i) {
		    cout << sorted[i].first << " " << sorted[i].second << '\n';
		}
	}
}