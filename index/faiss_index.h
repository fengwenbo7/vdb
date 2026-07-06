#pragma once
#include <vector>
#include "faiss/Index.h"
#include "roaring/roaring.h"

//定义RoaringBitmapIDSelector结构体，继承自faiss::IDSelector，用于属性过滤，在搜索时作为搜索参数
struct RoaringBitmapIDSelector:faiss::IDSelector{
    RoaringBitmapIDSelector(const roaring_bitmap_t* bitmap):bitmap_(bitmap){};
    bool is_member(int64_t id) const final{
        return roaring_bitmap_contains(bitmap_,id);
    }
    ~RoaringBitmapIDSelector() override{}
    const roaring_bitmap_t* bitmap_;
};

class FaissIndex{
public:
    FaissIndex(faiss::Index* index);
    void insert_vectors(const std::vector<float>& data,uint64_t label);
    void remove_vectors(const std::vector<faiss::idx_t>& ids);
    std::pair<std::vector<int64_t>,std::vector<float>> search_vectors(const std::vector<float>& query,int k,const roaring_bitmap_t* bitmap=nullptr);
private:
    faiss::Index* index_;
};