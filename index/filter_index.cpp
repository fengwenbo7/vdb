#include "filter_index.h"
#include <algorithm>
#include <set>
#include <memory>

FilterIndex::FilterIndex(){}

void FilterIndex::addIntFieldFilter(const std::string& field_name,int64_t value,uint64_t id){
    auto bitmap=roaring_bitmap_create();
    //把指定id写入位图中，表示该id对应的文档满足这个过滤条件
    roaring_bitmap_add(bitmap,id);
    intFieldFilter_[field_name][value]=bitmap;//将位图存入字段-值映射中
}
void FilterIndex::updateIntFieldFilter(const std::string& field_name,int64_t* old_value,int64_t new_value,uint64_t id){
    auto int_field_filter=intFieldFilter_.find(field_name);
    if(int_field_filter!=intFieldFilter_.end()){//已经有这个字段的位图
        auto value_map=intFieldFilter_[field_name];
        auto old_value_bitmap=old_value==nullptr?value_map.end():value_map.find(*old_value);
        if(old_value_bitmap!=value_map.end()){
            //有这个value的位图，从这个位图中把这个id删掉，表示这个id对应的文档不满足field_name->old_value的过滤条件
            auto old_bitmap=old_value_bitmap->second;
            roaring_bitmap_remove(old_bitmap,id);
        }

        //查找新值位图，没有的话就添加一个
        auto new_value_bitmap=value_map.find(new_value);
        if(new_value_bitmap==value_map.end()){
            auto new_bitmap=roaring_bitmap_create();
            value_map[new_value]=new_bitmap;
            new_value_bitmap=value_map.find(new_value);
        }

        roaring_bitmap_add(new_value_bitmap->second,id);//将这个id写入位图中，表示这个id对应的文档满足field_name->new_value的过滤条件
    }else{//没有这个字段的位图
        addIntFieldFilter(field_name,new_value,id);
    }
}

///找到所有满足要求的id集合
void FilterIndex::getIntFieldFilterBitmap(const std::string& field_name,Operation op,int64_t value,roaring_bitmap_t* result_bitmap){
    auto it=intFieldFilter_.find(field_name);
    if(it!=intFieldFilter_.end()){
        auto& value_map=it->second;
        if(op==Operation::EQUAL){
            auto bitmap_it=value_map.find(value);
            if(bitmap_it!=value_map.end()){
                roaring_bitmap_or_inplace(result_bitmap,bitmap_it->second);//or_inplace按位或，将搜索到的id与result与一下，从而得出满足要求的文档集合
            }
        }else if(op==Operation::NOT_EQUAL){
            for(const auto& entry:value_map){
                if(entry.first!=value){
                    roaring_bitmap_or_inplace(result_bitmap,entry.second);
                }
            }
        }
    }
}