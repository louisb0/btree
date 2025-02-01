#pragma once

class tree_base {
public:
    tree_base() = default;
    virtual ~tree_base() = 0;

    virtual int lower_bound(int target) const noexcept = 0;
};

inline tree_base::~tree_base() = default;
