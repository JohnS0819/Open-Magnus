#ifndef QUADTREE_HPP
#define QUADTREE_HPP
#include "Grid.hpp"
#include <unordered_set>
#include <vector>
#include <map>
#include <algorithm>

class QuadTree {
public:
	union Truncated_Rectangle {
		uint32_t Expressable;
		char corners[4];
	};

	template<size_t n>
	class Node {
	public:
		void collect(std::unordered_set<int>& set) const {
			for (auto& i : rectangles) {
				set.insert(i);
			}
			LL->collect(set);
			LR->collect(set);
			UL->collect(set);
			UR->collect(set);

		}
		~Node() {
			delete LL;
			delete LR;
			delete UL;
			delete UR;
		}
		void query(const uint32_t& rect,std::unordered_set<int>& set) const {
			//we were tricked
			if (!((rect & (255 << 16)) && (rect & (255 << 24)))) {
				return;
			}

			constexpr uint32_t FULL = (1 << (16 + n)) + (1 << (24 + n));

			for (auto& i : rectangles) {
				set.insert(i);
			}

			if (rect == FULL) {
				LL->collect(set);
				LR->collect(set);
				UL->collect(set);
				UR->collect(set);
				return;
			}

			constexpr uint32_t lower = 0;
			constexpr uint32_t upper = 1 << (n - 1);
			constexpr uint32_t mask = (upper | (upper << 8) | (upper << 16) | (upper << 24));
			uint32_t next = rect >> 1;
			uint32_t temp = (next & ((upper << 16) | (upper << 24)));
			uint32_t logic = temp | (mask & rect);
			constexpr uint32_t nextMask = (1 << n) - 1;
			constexpr uint32_t currentMask = (1 << (n + 1)) - 1;

			switch (logic)
			{
			case(lower):
				//std::cout << "only lower left quadrant is occupied" << std::endl;
				LL->query(rect, set);
				break;
			case((upper << 16) | (upper << 24)):
				//std::cout << "every quadrant is occupied" << std::endl;
				LL->query((rect & (nextMask | (nextMask << 8))) | ((upper << 16) | (upper << 24)), set);
				LR->query((rect & ((currentMask << 16) | (nextMask << 8))) + ((upper << 24) - (upper << 16)), set);
				UL->query((rect & ((nextMask) | (currentMask << 24))) - ((upper << 24) - (upper << 16)), set);
				UR->query((rect & ((currentMask << 16) | (currentMask << 24))) - ((upper << 24) + (upper << 16)), set);
				break;
			case((upper << 16)):
				//std::cout << "lower left and lower right are occupied" << std::endl;
				LL->query((rect & (nextMask | (nextMask << 8) | (nextMask << 24))) | (upper << 16), set);
				LR->query((rect & ((nextMask << 8) | (currentMask << 16) | (nextMask << 24))) - (upper << 16), set);
				break;

			case(upper << 24):
				//std::cout << "lower left and upper left are occupied" << std::endl;
				LL->query((rect & (nextMask | (nextMask << 8) | (nextMask << 16))) | (upper << 24), set);
				UL->query((rect & (nextMask | (nextMask << 16) | (currentMask << 24))) - (upper << 24), set);
				break;
			case(upper | upper << 16):
				//std::cout << "only lower right is ocupied" << std::endl;
				LR->query(rect - ((upper) | (upper << 16)), set);
				break;
			case((upper) | (upper << 8) | (upper << 16) | (upper << 24)):
				//std::cout << "only upper right is occupied" << std::endl;
				UR->query(rect - mask, set);
				break;
			case(upper | (upper << 16) | (upper << 24)):
				//std::cout << "only lower right and upper right are occupied" << std::endl;
				LR->query((rect & (nextMask | (nextMask << 8) | (currentMask << 16))) + ((upper << 24) - upper - (upper << 16)), set);
				UR->query((rect & ((nextMask) | (currentMask << 16) | (currentMask << 24))) - ((upper << 24) + upper + (upper << 16)), set);
				break;
			case((upper << 8) | (upper << 16) | (upper << 24)):
				//std::cout << "only upper left and upper right are occupied" << std::endl;
				UL->query((rect & (nextMask | (nextMask << 8) | (currentMask << 24))) - ((upper << 8) + (upper << 24) - (upper << 16)), set);
				UR->query((rect & ((nextMask << 8) | (currentMask << 16) | (currentMask << 24))) - ((upper << 24) + (upper << 8) + (upper << 16)), set);
				break;

			default:
				//std::cout << "only upper left is occupied" << std::endl;
				UL->query(rect - ((upper << 8) | (upper << 24)), set);
				break;
			}
		}


		std::vector<int> rectangles;
		Node<n - 1>* LL;
		Node<n - 1>* LR;
		Node<n - 1>* UL;
		Node<n - 1>* UR;

		Node() {
			LL = new Node<n - 1>;
			UL = new Node<n - 1>;
			UR = new Node<n - 1>;
			LR = new Node<n - 1>;

		}

		void insert(const uint32_t& rect, const int& id) {
			//we were tricked
			if (!((rect & (255 << 16)) && (rect & (255 << 24)))) {
				return;
			}

			constexpr uint32_t FULL = (1 << (16 + n)) + (1 << (24 + n));

			if (rect == FULL) {
				rectangles.push_back(id);
				return;
			}

			constexpr uint32_t lower = 0;
			constexpr uint32_t upper = 1 << (n - 1);
			constexpr uint32_t mask = (upper | (upper << 8) | (upper << 16) | (upper << 24));
			uint32_t next = rect >> 1;
			uint32_t temp = (next & ((upper << 16) | (upper << 24)));
			uint32_t logic = temp | (mask & rect);
			constexpr uint32_t nextMask = (1 << n) - 1;
			constexpr uint32_t currentMask = (1 << (n + 1)) - 1;

			switch (logic)
			{
			case(lower):
				//std::cout << "only lower left quadrant is occupied" << std::endl;
				LL->insert(rect, id);
				break;
			case((upper << 16) | (upper << 24)):
				//std::cout << "every quadrant is occupied" << std::endl;
				LL->insert((rect & (nextMask | (nextMask << 8)) | ((upper << 16) | (upper << 24))), id);
				LR->insert((rect & ((currentMask << 16) | (nextMask << 8))) + ((upper << 24) - (upper << 16)), id);
				UL->insert((rect & ((nextMask) | (currentMask << 24))) - ((upper << 24) - (upper << 16)), id);
				UR->insert((rect & ((currentMask << 16) | (currentMask << 24))) - ((upper << 24) + (upper << 16)), id);
				break;
			case((upper << 16)):
				//std::cout << "lower left and lower right are occupied" << std::endl;
				LL->insert((rect & (nextMask | (nextMask << 8) | (nextMask << 24))) | (upper << 16), id);
				LR->insert((rect & ((nextMask << 8) | (currentMask << 16) | (nextMask << 24))) - (upper << 16), id);
				break;

			case(upper << 24):
				//std::cout << "lower left and upper left are occupied" << std::endl;
				LL->insert((rect & (nextMask | (nextMask << 8) | (nextMask << 16))) | (upper << 24), id);
				UL->insert((rect & (nextMask | (nextMask << 16) | (currentMask << 24))) - (upper << 24), id);
				break;
			case(upper | upper << 16):
				//std::cout << "only lower right is ocupied" << std::endl;
				LR->insert(rect - ((upper) | (upper << 16)), id);
				break;
			case((upper) | (upper << 8) | (upper << 16) | (upper << 24)):
				//std::cout << "only upper right is occupied" << std::endl;
				UR->insert(rect - mask, id);
				break;
			case(upper | (upper << 16) | (upper << 24)):
				//std::cout << "only lower right and upper right are occupied" << std::endl;
				LR->insert((rect & (nextMask | (nextMask << 8) | (currentMask << 16))) + ((upper << 24) - upper - (upper << 16)), id);
				UR->insert((rect & ((nextMask) | (currentMask << 16) | (currentMask << 24))) - ((upper << 24) + upper + (upper << 16)), id);
				break;
			case((upper << 8) | (upper << 16) | (upper << 24)):
				//std::cout << "only upper left and upper right are occupied" << std::endl;
				UL->insert((rect & (nextMask | (nextMask << 8) | (currentMask << 24))) - ((upper << 8) + (upper << 24) - (upper << 16)), id);
				UR->insert((rect & ((nextMask << 8) | (currentMask << 16) | (currentMask << 24))) - ((upper << 24) + (upper << 8) + (upper << 16)), id);
				break;

			default:
				//std::cout << "only upper left is occupied" << std::endl;
				UL->insert(rect - ((upper << 8) | (upper << 24)), id);
				break;
			}
		}

		void clear() {
			rectangles.resize(0);
			LL->clear();
			LR->clear();
			UL->clear();
			UR->clear();

		}
	};

	template<>
	class Node<0> {
	public:
		void clear() {
			rectangles.resize(0);
		}

		void collect(std::unordered_set<int>& set) {
			for (auto& i : rectangles) {
				set.insert(i);
			}
		}
		std::vector<int> rectangles;
		void insert(const uint32_t& rect, const int& id) {
			if (!((rect & (255 << 16)) && (rect & (255 << 24)))) {
				return;
			}

			rectangles.push_back(id);

		}

		void query(const uint32_t& rect, std::unordered_set<int>& set) {
			if (!((rect & (255 << 16)) && (rect & (255 << 24)))) {
				return;
			}
			for (auto& i : rectangles) {
				set.insert(i);
			}
		}

	};


	Node<4>* Root;
	QuadTree() {
		Root = new Node<4>;
	}
	void insert(const uint32_t& rect, const int& id) {
		Root->insert(rect, id);
	}
	void clear() {
		Root->clear();
	}

	std::unordered_set<int> query(const uint32_t& rect) {
		std::unordered_set<int> return_value;
		Root->query(rect, return_value);
		return return_value;
	}

	~QuadTree() {
		delete Root;
	}

};


//class Collider {
//public:
//	struct rectangle {
//		int LX, LY, UX, UY;
//	};
//
//
//	std::map<int, rectangle> index;
//	std::vector<std::pair<rectangle, rectangle>> output;
//	void insert(rectangle A) {
//		index.insert({ index.size(),A });
//	}
//
//	void stab(std::vector<rectangle> A, std::vector<rectangle> B) {
//		int i = 0;
//		int j = 0;
//		while (i < A.size() && j < B.size()) {
//			if (A[i].LY < B[j].LY) {
//				int k = j;
//				while (k < B.size() && B[k].LY < A[i].UY) {
//					output.push_back({ A[i],B[k] });
//					++k;
//				}
//				++i;
//			}
//			else {
//				int k = i;
//				while (k < A.size() && A[k].LY < B[j].UY) {
//					output.push_back({ A[k],B[j] });
//					++k;
//				}
//				++j;
//			}
//		}
//	}
//
//
//	void detect(std::vector<std::pair<int,int>> V, std::vector<int> H) {
//		int m = V.size() / 2;
//		if (m == 0) {
//			return;
//		}
//		int mid = V[m].first;
//		int mid_index = V[m].second;
//		int max = V[V.size() - 1].first;
//		int min = V[0].first;
//		std::vector<std::pair<int, int>> V1(V.begin(), V.begin() + m);
//		std::vector<std::pair<int, int>> V2(V.begin() + m, V.end());
//		std::vector<rectangle> S11, S12, S22, S21;
//		for (int i = 0; i < V1.size(); ++i) {
//			auto interval = V1[i];
//			int index_ = interval.second;
//			if (index[index_].LX < interval.first) {
//				S11.push_back(index[index_]);
//			}
//			else if (index[index_].UX > max) {
//				S12.push_back(index[index_]);
//			}
//		}
//		for (int i = 0; i < V2.size(); ++i) {
//			auto interval = V2[i];
//			int index_ = interval.second;
//			if (index[index_].UX > interval.first) {
//				S22.push_back(index[index_]);
//			}
//			else if (index[index_].LX < min) {
//				S21.push_back(index[index_]);
//			}
//		}
//		auto cmp = [](const rectangle& a, const rectangle& b) {return a.LY < b.LY; };
//		std::sort(S11.begin(), S11.end(), cmp);
//		std::sort(S12.begin(), S12.end(), cmp);
//		std::sort(S22.begin(), S22.end(), cmp);
//		std::sort(S21.begin(), S21.end(), cmp);
//		
//		
//		stab(S12, S22);
//		stab(S21, S11);
//		stab(S12, S21);
//
//		detect(V1, H);
//		detect(V2, H);
//
//	}
//
//
//	void intersect() {
//		std::vector<std::pair<int, int>> V;
//		std::vector<int> H;
//
//
//		for (int i = 0; i < index.size(); ++i) {
//
//			V.push_back({ index[i].LX,i });
//			V.push_back({ index[i].UX,i });
//		}
//		std::sort(V.begin(), V.end());
//
//		detect(V, H);
//
//	}

//
//};








#endif