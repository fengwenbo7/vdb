#pragma once
#include <vector>
#include "hnswlib/hnswlib.h"
#include "roaring/roaring.h"

class RoaringBitmapIDFilter:public hnswlib::BaseFilterFunctor{
public:
    RoaringBitmapIDFilter(const roaring_bitmap_t* bitmap){}
    bool operator()(hnswlib::labeltype id) { return roaring_bitmap_contains(bitmap_,static_cast<uint32_t>(id)); }

private:
    const roaring_bitmap_t* bitmap_;
};

class HNSWLIBIndex{
public:
    /// @brief 分层可导航小世界地图索引构造函数
    /// @param dim 维度
    /// @param num_data 预计存储的向量个数 
    /// @param m 节点最大连接数
    /// @param ef_construction 构建索引时动态候选列表大小
    HNSWLIBIndex(int dim,int num_data,int m,int ef_construction=200);
    void insert_vectors(const std::vector<float>& data,uint64_t label);

    /// @brief 搜索向量
    /// @param query 待搜索向量
    /// @param k top K
    /// @param ef_search 搜索时候选数，即查询时每层保留的候选节点数 
    /// @return 
    std::pair<std::vector<int64_t>,std::vector<float>> search_vectors(const std::vector<float>& query,int k,const roaring_bitmap_t* bitmap=nullptr,int ef_search=50);

private:
    hnswlib::HierarchicalNSW<float>* index_;
};