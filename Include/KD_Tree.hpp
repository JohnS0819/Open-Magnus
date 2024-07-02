//MIT License
//
//Copyright(c) 2017 gishi523
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this softwareand associated documentation files(the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions :
//
//The above copyright noticeand this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.
//


//not my code
#ifndef KD_TREE_HPP
#define KD_TREE_HPP

#include <vector>
#include <numeric>
#include <algorithm>
#include <exception>
#include <functional>

namespace kdt
{
	/** @brief k-d tree class.
	*/
	template <class PointT>
	class KDTree
	{
	public:

		KDTree(const std::vector<PointT>& points) : root_(nullptr), points_(points) { build(); }

		/** @brief The destructor.
		*/
		~KDTree() { clear(); }

		/** @brief Re-builds k-d tree.
		*/
		void build()
		{
			clear();

			//points_ = points;

			std::vector<int> indices(points_.size());
			std::iota(std::begin(indices), std::end(indices), 0);

			root_ = buildRecursive(indices.data(), (int)points_.size(), 0);
		}

		/** @brief Clears k-d tree.
		*/
		void clear()
		{
			clearRecursive(root_);
			root_ = nullptr;
			//points_.clear();
		}

		/** @brief Validates k-d tree.
		*/
		bool validate() const
		{
			try
			{
				validateRecursive(root_, 0);
			}
			catch (const Exception&)
			{
				return false;
			}

			return true;
		}

		/** @brief Searches the nearest neighbor.
		*/
		int nnSearch(const PointT& query, float* minDist = nullptr) const
		{
			int guess;
			float _minDist = std::numeric_limits<float>::max();

			nnSearchRecursive(query, root_, &guess, &_minDist);

			if (minDist)
				*minDist = _minDist;

			return guess;
		}

		/** @brief Searches k-nearest neighbors.
		*/
		std::vector<int> knnSearch(const PointT& query, int k) const
		{
			KnnQueue queue(k);
			knnSearchRecursive(query, root_, queue, k);

			std::vector<int> indices(queue.size());
			for (size_t i = 0; i < queue.size(); i++)
				indices[i] = queue[i].second;

			return indices;
		}

		/** @brief Searches neighbors within radius.
		*/
		template<typename point_like>
		std::vector<int> radiusSearch(const point_like& query, float radius) const
		{
			std::vector<int> indices;
			radiusSearchRecursive(query, root_, indices, radius);
			return indices;
		}

	protected:

		/** @brief k-d tree node.
		*/
		struct Node
		{
			int idx;       //!< index to the original point
			Node* next[2]; //!< pointers to the child nodes
			int axis;      //!< dimension's axis

			Node() : idx(-1), axis(-1) { next[0] = next[1] = nullptr; }
		};

		/** @brief k-d tree exception.
		*/
		class Exception : public std::exception { using std::exception::exception; };

		/** @brief Bounded priority queue.
		*/
		template <class T, class Compare = std::less<T>>
		class BoundedPriorityQueue
		{
		public:

			BoundedPriorityQueue() = delete;
			BoundedPriorityQueue(size_t bound) : bound_(bound) { elements_.reserve(bound + 1); };

			void push(const T& val)
			{
				auto it = std::find_if(std::begin(elements_), std::end(elements_),
					[&](const T& element) { return Compare()(val, element); });
				elements_.insert(it, val);

				if (elements_.size() > bound_)
					elements_.resize(bound_);
			}

			const T& back() const { return elements_.back(); };
			const T& operator[](size_t index) const { return elements_[index]; }
			size_t size() const { return elements_.size(); }

		protected:
			size_t bound_;
			std::vector<T> elements_;
		};

		/** @brief Priority queue of <distance, index> pair.
		*/
		using KnnQueue = BoundedPriorityQueue<std::pair<float, int>>;

		/** @brief Builds k-d tree recursively.
		*/
		Node* buildRecursive(int* indices, int npoints, int depth)
		{
			if (npoints <= 0)
				return nullptr;

			const int axis = depth % PointT::DIM;
			const int mid = (npoints - 1) / 2;

			std::nth_element(indices, indices + mid, indices + npoints, [&](int lhs, int rhs)
				{
					return points_[lhs][axis] < points_[rhs][axis];
				});

			Node* node = new Node();
			node->idx = indices[mid];
			node->axis = axis;

			node->next[0] = buildRecursive(indices, mid, depth + 1);
			node->next[1] = buildRecursive(indices + mid + 1, npoints - mid - 1, depth + 1);

			return node;
		}

		/** @brief Clears k-d tree recursively.
		*/
		void clearRecursive(Node* node)
		{
			if (node == nullptr)
				return;

			if (node->next[0])
				clearRecursive(node->next[0]);

			if (node->next[1])
				clearRecursive(node->next[1]);

			delete node;
		}

		/** @brief Validates k-d tree recursively.
		*/
		void validateRecursive(const Node* node, int depth) const
		{
			if (node == nullptr)
				return;

			const int axis = node->axis;
			const Node* node0 = node->next[0];
			const Node* node1 = node->next[1];

			if (node0 && node1)
			{
				if (points_[node->idx][axis] < points_[node0->idx][axis])
					throw Exception();

				if (points_[node->idx][axis] > points_[node1->idx][axis])
					throw Exception();
			}

			if (node0)
				validateRecursive(node0, depth + 1);

			if (node1)
				validateRecursive(node1, depth + 1);
		}

		template<typename T1,typename T2>
		static float distance(const T1& p, const T2& q)
		{
			float dist = 0;
			for (size_t i = 0; i < PointT::DIM; i++)
				dist += (p[i] - q[i]) * (p[i] - q[i]);
			return sqrt(dist);
		}

		/** @brief Searches the nearest neighbor recursively.
		*/
		void nnSearchRecursive(const PointT& query, const Node* node, int* guess, float* minDist) const
		{
			if (node == nullptr)
				return;

			const PointT& train = points_[node->idx];

			const float dist = distance(query, train);
			if (dist < *minDist)
			{
				*minDist = dist;
				*guess = node->idx;
			}

			const int axis = node->axis;
			const int dir = query[axis] < train[axis] ? 0 : 1;
			nnSearchRecursive(query, node->next[dir], guess, minDist);

			const float diff = fabs(query[axis] - train[axis]);
			if (diff < *minDist)
				nnSearchRecursive(query, node->next[!dir], guess, minDist);
		}

		/** @brief Searches k-nearest neighbors recursively.
		*/
		void knnSearchRecursive(const PointT& query, const Node* node, KnnQueue& queue, int k) const
		{
			if (node == nullptr)
				return;

			const PointT& train = points_[node->idx];

			const float dist = distance(query, train);
			queue.push(std::make_pair(dist, node->idx));

			const int axis = node->axis;
			const int dir = query[axis] < train[axis] ? 0 : 1;
			knnSearchRecursive(query, node->next[dir], queue, k);

			const float diff = fabs(query[axis] - train[axis]);
			if ((int)queue.size() < k || diff < queue.back().first)
				knnSearchRecursive(query, node->next[!dir], queue, k);
		}

		/** @brief Searches neighbors within radius.
		*/
		template<typename point_like>
		void radiusSearchRecursive(const point_like& query, const Node* node, std::vector<int>& indices, float radius) const
		{
			if (node == nullptr)
				return;

			const PointT& train = points_[node->idx];

			const float dist = distance(query, train);
			if (dist < radius)
				indices.push_back(node->idx);

			const int axis = node->axis;
			const int dir = query[axis] < train[axis] ? 0 : 1;
			radiusSearchRecursive(query, node->next[dir], indices, radius);

			const float diff = fabs(query[axis] - train[axis]);
			if (diff < radius)
				radiusSearchRecursive(query, node->next[!dir], indices, radius);
		}

		Node* root_;                 //!< root node
		const std::vector<PointT> &points_; //!< points
	};
} // kdt

#endif // !KD_TREE_HPP