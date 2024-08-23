#pragma once

template <typename... ComponentTypes> class sparse_set_view {
      public:
	sparse_set_view() = default;
	sparse_set_view(sparse_set_view&&) = delete;
	sparse_set_view(const sparse_set_view&) = delete;
	sparse_set_view& operator=(sparse_set_view&&) = delete;
	sparse_set_view& operator=(const sparse_set_view&) = delete;
	~sparse_set_view() = default;

      private:
};
