#ifndef COLLISION_PRUNING_HPP
#define COLLISION_PRUNING_HPP

#include <vector>
#include <algorithm>
#include <map>
#include <unordered_set>

class Collider {
public:
	struct rectangle {
		int LX, LY, UX, UY;
	};
private:
	struct x_interval{
		int value;
		int complement;
		int index;

		bool operator<(const x_interval& other) const {
			if (value < other.value) {
				return true;
			}
			if (value > other.value) {
				return false;
			}
			
			return complement < other.complement;
		};
	};
	struct linked_y_interval {
		int LY;
		int UY;
		x_interval *linkage;
		bool operator<(const linked_y_interval& other) const {
			return LY < other.LY;
		};
	};

	struct indexed_y_interval {
		int LY;
		int UY;
		int index;
	};


	std::vector<rectangle> list_of_rectangles;

private:
	std::vector<x_interval>V_;
	std::vector<linked_y_interval> H_;
	std::vector<linked_y_interval> H_buffer;
	std::vector<indexed_y_interval> S11_, S12_, S22_, S21_;
public:
	std::vector<std::unordered_set<int>> output;


	void insert(rectangle A) {
		output.push_back({});
		list_of_rectangles.push_back(A);
	}

	void stab(indexed_y_interval *A, indexed_y_interval *B,const int& A_size, const int& B_size) {
		int i = 0;
		int j = 0;
		while (i < A_size && j < B_size) {
			int i1 = A[i].index;
			int i2 = B[j].index;
			if (A[i].LY < B[j].LY) {
				int k = j;
				while (k < B_size && B[k].LY < A[i].UY) {
					if (A[i].index == B[k].index) {
						++k;
						continue;
					}
					if (A[i].index < B[k].index) {
						output[A[i].index].insert(B[k].index);
					}
					else {
						output[B[k].index].insert(A[i].index);
					}
					++k;
				}
				++i;
			}
			else {
				int k = i;
				while (k < A_size && A[k].LY < B[j].UY) {
					if (A[k].index == B[j].index) {
						++k;
						continue;
					}
					if (A[k].index < B[j].index) {
						output[A[k].index].insert(B[j].index);
					}
					else {
						output[B[j].index].insert(A[k].index);
					}
					++k;
				}
				++j;
			}
		}
	}
	void detect(x_interval *V, linked_y_interval *H, linked_y_interval *consumable_,int m) {
		if (m < 2) {
			return;
		}
		int next_m = m >> 1;
		int mid = V[next_m].value;
		int max = V[m - 1].value;
		int min = V[0].value;
		auto* mid_ptr = V + next_m;
		int mid_value = mid_ptr->value;
		auto *Right_H = consumable_ + next_m;
		auto *Left_H = consumable_;
		int S11_size = 0;
		int S12_size = 0;
		int S22_size = 0;
		int S21_size = 0;
		auto *S11 = S11_.data();
		auto *S12 = S12_.data();
		auto *S22 = S22_.data();
		auto *S21 = S21_.data();

		for (int i = 0; i < m; ++i) {
			if (H[i].linkage < mid_ptr) {
				*Left_H = H[i];
				++Left_H;
				auto x_value = *H[i].linkage;
				//right edge of rectangle on left side of midpoint
				if (x_value.value > x_value.complement) {
					S11[S11_size] = {H[i].LY,H[i].UY,x_value.index};
					++S11_size;
				}
				//left edge of rectangle with right edge greater than maximum
				else if (x_value.complement > max) {
					S12[S12_size] = { H[i].LY,H[i].UY,x_value.index };
					++S12_size;
				}
			}
			else {
				*Right_H = H[i];
				++Right_H;
				auto x_value = *H[i].linkage;
				//left edge of rectangle on right side of midpoint
				if (x_value.value < x_value.complement) {
					S22[S22_size] = {H[i].LY,H[i].UY,x_value.index};
					++S22_size;
				}
				//right edge of rectangle with left edge less than mid;
				else if (x_value.complement < min) {
					S21[S21_size] = { H[i].LY,H[i].UY,x_value.index };
					++S21_size;
				}
			}


		}
		stab(S12, S22, S12_size, S22_size);
		stab(S12, S21, S12_size, S21_size);
		stab(S21, S11, S21_size, S11_size);
		detect(V, consumable_, H, next_m);
		detect(V + next_m, consumable_ + next_m, H + next_m, m - next_m);
	}


	void intersect() {
		V_.reserve(2 * list_of_rectangles.size());
		H_.reserve(2 * list_of_rectangles.size());
		S11_.resize(list_of_rectangles.size());
		S12_.resize(list_of_rectangles.size());
		S21_.resize(list_of_rectangles.size());
		S22_.resize(list_of_rectangles.size());
		H_buffer.resize(2 * list_of_rectangles.size());

		std::vector<int> adjacency_map(list_of_rectangles.size());

		for (int i = 0; i < list_of_rectangles.size(); ++i) {
			V_.push_back({ list_of_rectangles[i].LX,list_of_rectangles[i].UX , i });
			V_.push_back({ list_of_rectangles[i].UX,list_of_rectangles[i].LX , i });
			H_.push_back({ list_of_rectangles[i].LY,list_of_rectangles[i].UY , NULL });
			H_.push_back({ list_of_rectangles[i].LY,list_of_rectangles[i].UY , NULL });
		}
		std::sort(V_.begin(), V_.end());
		for (int i = 0; i < V_.size(); ++i) {
			if (V_[i].complement > V_[i].value) {
				adjacency_map[V_[i].index] = i;
			}
			int index = 2 * V_[i].index;
			if (H_[index].linkage != NULL) {
				H_[index + 1].linkage = V_.data() + i;
			}
			else {
				H_[index].linkage = V_.data() + i;
			}
		}
		int offset = 0;
		for (int i = 1; i < V_.size(); ++i) {
			if (offset + V_[i].value == V_[i - 1].value) {
				++offset;
			}
			if (V_[i].complement < V_[i].value) {
				V_[adjacency_map[V_[i].index]].complement += offset;
				V_[i].complement = V_[adjacency_map[V_[i].index]].value;
			}
			V_[i].value += offset;
		}
		std::sort(H_.begin(), H_.end());
		detect(V_.data(), H_.data(),H_buffer.data(),V_.size());

	}


};

#endif