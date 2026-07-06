#pragma once

#include <vector>
#include <map>
#include <string>
#include <set>
#include <memory>
#include "roaring/roaring.h"

class FilterIndex{
public:
    enum class Operation{
        EQUAL,
        NOT_EQUAL
    };
    FilterIndex();
    void addIntFieldFilter(const std::string& field_name,int64_t value,uint64_t id);
    void updateIntFieldFilter(const std::string& field_name,int64_t* old_value,int64_t new_value,uint64_t id);
    void getIntFieldFilterBitmap(const std::string& field_name,Operation op,int64_t value,roaring_bitmap_t* result_bitmap);

private:
    //<文档字段，字段值，位图>
    std::map<std::string,std::map<long,roaring_bitmap_t*>> intFieldFilter_;
};