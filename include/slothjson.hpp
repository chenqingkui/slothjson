#ifndef __slothjson_hpp_h__
#define __slothjson_hpp_h__

#ifdef WIN32
#pragma warning(disable: 4305)
#pragma warning(disable: 4996)
#endif // WIN32

#include <stdint.h>
#include <limits.h>
#include <float.h>
#include <typeinfo>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <fstream>
#include <iostream>
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/error/error.h"
#include "rapidjson/error/en.h"
namespace slothjson
{
    typedef rapidjson::Document::AllocatorType allocator_t;
	template <bool pretty> struct json_writer_t;

    template <>
    struct json_writer_t <true>
    {
        typedef rapidjson::PrettyWriter<rapidjson::StringBuffer> writer_t;
    };

    template <>
    struct json_writer_t <false>
    {
        typedef rapidjson::Writer<rapidjson::StringBuffer> writer_t;
    };

}

namespace slothjson
{
	class slothjson_cxx
	{
	public:

		template<typename T> inline static bool encode(const T& obj_val, allocator_t& alloc, rapidjson::Value& json_val);
		template<typename T> inline static bool decode(const rapidjson::Value& json_val, T& obj_val);
		
		template<typename T>  inline static 
		bool encode(const std::vector<T>& obj_val, allocator_t& alloc, rapidjson::Value& json_val);
		template<typename T>  inline static 
		bool decode(const rapidjson::Value& json_val, std::vector<T>& obj_val);

		template<typename T>  inline static 
		bool encode(const std::map<std::string, T>& obj_val, allocator_t& alloc, rapidjson::Value& json_val);
		template<typename T>  inline static 
		bool decode(const rapidjson::Value& json_val, std::map<std::string, T>& obj_val);

		template<typename T>  inline static 
		bool encode_field(const T& field, const char * field_name, allocator_t& alloc, rapidjson::Value& json_val);
		template<typename T>  inline static 
		bool decode_field(const rapidjson::Value& json_val, const char * field_name, T& field, bool& field_in_json);

		template<bool pretty, typename T>  inline static 
		bool encode(const T& obj_val, std::string& json_val);
		template<typename T>  inline static 
		bool decode(const std::string& json_val, T& obj_val);

		template<bool pretty, typename T>  inline static 
		bool dump(const T& obj_val, const char * path);

		template<typename T>   inline static
		bool load(const char * path, T& obj_val);
		
		inline static bool encode_document(rapidjson::Value& src, rapidjson::Document& dst);
		inline static bool decode_document(rapidjson::Document& src, rapidjson::Value& dst);

		template <typename src_t, typename dst_t>  inline static 
		bool __convert(const src_t& src, dst_t& dst);

		template<bool pretty>  inline static 
		bool encode(const rapidjson::Document& doc, std::string& json_val);

		template<typename T> inline static
		bool decode(const char * json_val, T& obj_val);
	};
};

// implement
namespace slothjson
{

	template<typename T> 
	bool slothjson_cxx::encode(const T& obj_val, allocator_t& alloc, rapidjson::Value& json_val)
	{
		return obj_val.encode(alloc,json_val);
	}
		
	template<typename T>  
	bool slothjson_cxx::decode(const rapidjson::Value& json_val, T& obj_val)
	{
		return obj_val.decode(json_val);
	}

    template<typename T>
    bool slothjson_cxx::encode(const std::map<std::string, T>& obj_val, allocator_t& alloc, rapidjson::Value& json_val)
    {
        json_val.SetObject();
        typename std::map<std::string, T>::const_iterator it;
        for (it = obj_val.begin(); it != obj_val.end(); ++it)
        {
            rapidjson::Value val;
            if (!encode(it->second, alloc, val))
            {
                return false;
            }
            json_val.AddMember(rapidjson::StringRef(it->first.c_str()), val.Move(), alloc);
        }
        return true;
    }
    template<typename T>
    bool slothjson_cxx::decode(const rapidjson::Value& json_val, std::map<std::string, T>& obj_val)
    {
        if (!json_val.IsObject())
        {
            return false;
        }
        for (rapidjson::Value::ConstMemberIterator it = json_val.MemberBegin(); it != json_val.MemberEnd(); ++it)
        {
            T val;
            if (!decode(it->value, val))
            {
                return false;
            }
            obj_val[it->name.GetString()] = val;
        }
        return true;
    }

    template<typename T>
    bool slothjson_cxx::encode(const std::vector<T>& obj_val, allocator_t& alloc, rapidjson::Value& json_val)
    {
        json_val.SetArray();
        typename std::vector<T>::const_iterator it;
        for (it = obj_val.begin(); it != obj_val.end(); ++it)
        {
            rapidjson::Value temp;
            if (!encode(*it, alloc, temp))
            {
                return false;
            }
            json_val.PushBack(temp.Move(), alloc);
        }
        return true;
    }
    template<typename T>
    bool slothjson_cxx::decode(const rapidjson::Value& json_val, std::vector<T>& obj_val)
    {
        if (!json_val.IsArray())
        {
            return false;
        }
        rapidjson::SizeType size = json_val.Size();
        obj_val.reserve(size);

        for (rapidjson::SizeType i = 0; i < size; ++i)
        {
            T tmp_val;
            if (!decode(json_val[i], tmp_val))
            {
                return false;
            }
            obj_val.push_back(tmp_val);
        }
        return true;
    }

} // slothjson

namespace slothjson
{
    template<typename T>
    bool slothjson_cxx::encode_field(const T& field, const char * field_name, allocator_t& alloc, rapidjson::Value& json_val)
    {
        if (!field_name)
        {
            return false;
        }
        rapidjson::Value tmp_json_val;
        if (!encode(field, alloc, tmp_json_val))
        {
            return false;
        }
        rapidjson::Value key(field_name, alloc);
        json_val.AddMember(key, tmp_json_val.Move(), alloc);
        return true;
    }

    template<typename T>
    bool slothjson_cxx::decode_field(const rapidjson::Value& json_val, const char * field_name, T& field, bool& field_in_json)
    {
        field_in_json = false;
        if (!json_val.IsObject() || !field_name)
        {
            return false;
        }
        if (json_val.HasMember(field_name))
        {
            field_in_json = true;
            const rapidjson::Value &tmp_json_val = json_val[field_name];
            if (!decode(tmp_json_val, field))
            {
                return false;
            }
        }
        return true;
    }

} // slothjson

namespace slothjson
{
    
    template<bool pretty>
    bool slothjson_cxx::encode(const rapidjson::Document& doc, std::string& json_val)
    {
        rapidjson::StringBuffer buffer;
        typename json_writer_t<pretty>::writer_t writer(buffer);
        doc.Accept(writer);
        json_val = buffer.GetString();
        return true;
    }

    template<bool pretty, typename T>
    bool slothjson_cxx::encode(const T& obj_val, std::string& json_val)
    {
        rapidjson::Document doc;
        rapidjson::Value value;
        if (!encode(obj_val, doc.GetAllocator(), value))
        {
            return false;
        }
        if (!encode_document(value, doc))
        {
            return false;
        }
        return encode <pretty> (doc, json_val);
    }
    template<typename T>
    bool slothjson_cxx::decode(const char * json_val, T& obj_val)
    {
        rapidjson::Document doc;
        doc.Parse(json_val);
        rapidjson::Value value;
        if (!decode_document(doc, value))
        {
            return false;
        }
        return decode(value, obj_val);
    }
    template<typename T>
    bool slothjson_cxx::decode(const std::string& json_val, T& obj_val)
    {
        return decode(json_val.c_str(), obj_val);
    }

} // slothjson

namespace slothjson
{
    template<bool pretty, typename T>
    bool slothjson_cxx::dump(const T& obj_val, const char * path)
    {
        std::string json_val;
        if (!encode <pretty, T> (obj_val, json_val))
        {
            return false;
        }
        std::ofstream os;
        try
        {
            os.open(path, std::ios::binary);
        }
        catch (...)
        {
            return false;
        }
        os << json_val;
        os.close();
        return true;
    }

    struct json_loader_t
    {
        inline json_loader_t(const char * path);
        inline ~json_loader_t();
        inline const char * c_str();
		inline char * __load(const char * path);
    private:
        char * data_;
    };

    template<typename T>
    bool slothjson_cxx::load(const char * path, T& obj_val)
    {
        json_loader_t loader(path);
        if (!loader.c_str())
        {
            return false;
        }
        return decode(loader.c_str(), obj_val);
    }
};

namespace slothjson
{
    // bool
    template<> inline bool slothjson_cxx::encode(const bool& obj_val, allocator_t& alloc, rapidjson::Value& json_val)
    {
        json_val.SetBool(obj_val);
        return true;
    }
    template<> inline bool slothjson_cxx::decode(const rapidjson::Value& json_val, bool& obj_val)
    {
        if (json_val.IsBool())
        {
            obj_val = json_val.GetBool();
            return true;
        }
        else if (json_val.IsInt())
        {
            int tmp = json_val.GetInt();
            if (!tmp)
            {
                obj_val = false;
            }
            else
            {
                obj_val = true;
            }
            return true;
        }
        return false;
    }
    // std::string
    template<> inline bool slothjson_cxx::encode(const std::string& obj_val, allocator_t& alloc, rapidjson::Value& json_val)
    {
        json_val.SetString(rapidjson::StringRef(obj_val.c_str()));
        return true;
    }
    template<> inline bool slothjson_cxx::decode(const rapidjson::Value& json_val, std::string& obj_val)
    {
        if (json_val.IsString())
        {
            obj_val = json_val.GetString();
            return true;
        }
        return false;
    }
    // int32_t
    template<> inline bool slothjson_cxx::encode(const int32_t& obj_val, allocator_t& alloc, rapidjson::Value& json_val)
    {
        json_val.SetInt(obj_val);
        return true;
    }
    template<> inline bool slothjson_cxx::decode(const rapidjson::Value& json_val, int32_t& obj_val)
    {
        if (json_val.IsInt())
        {
            obj_val = json_val.GetInt();
            return true;
        }
        return false;
    }
    // uint32_t
    template<> inline bool slothjson_cxx::encode(const uint32_t& obj_val, allocator_t& alloc, rapidjson::Value& json_val)
    {
        json_val.SetUint(obj_val);
        return true;
    }
    template<> inline bool slothjson_cxx::decode(const rapidjson::Value& json_val, uint32_t& obj_val)
    {
        if (json_val.IsInt())
        {
            obj_val = json_val.GetInt();
            return true;
        }
        else if (json_val.IsUint())
        {
            obj_val = json_val.GetUint();
            return true;
        }
        return false;
    }
    // double
    template<> inline bool slothjson_cxx::encode(const double& obj_val, allocator_t& alloc, rapidjson::Value& json_val)
    {
        json_val.SetDouble(obj_val);
        return true;
    }
    template<> inline bool slothjson_cxx::decode(const rapidjson::Value& json_val, double & obj_val)
    {
        if (json_val.IsDouble())
        {
            obj_val = json_val.GetDouble();
            return true;
        }
        return false;
    }

    // int64_t & uint64_t
    template<> inline bool slothjson_cxx::encode(const int64_t& obj_val, allocator_t& alloc, rapidjson::Value& json_val)
    {
        json_val.SetInt64(obj_val);
        return true;
    }
    template<> inline bool slothjson_cxx::decode(const rapidjson::Value& json_val, int64_t& obj_val)
    {
        if (json_val.IsInt64())
        {
            obj_val = json_val.GetInt64();
            return true;
        }
        else if (json_val.IsInt())
        {
            obj_val = json_val.GetInt();
            return true;
        }
        else if (json_val.IsUint())
        {
            obj_val = json_val.GetUint();
            return true;
        }
        return false;
    }
    template<> inline bool slothjson_cxx::encode(const uint64_t& obj_val, allocator_t& alloc, rapidjson::Value& json_val)
    {
        json_val.SetUint64(obj_val);
        return true;
    }
    template<> inline bool slothjson_cxx::decode(const rapidjson::Value& json_val, uint64_t& obj_val)
    {
        if (json_val.IsUint64())
        {
            obj_val = json_val.GetUint64();
            return true;
        }
        else if (json_val.IsInt())
        {
            obj_val = json_val.GetInt();
            return true;
        }
        else if (json_val.IsUint())
        {
            obj_val = json_val.GetUint();
            return true;
        }
        return false;
    }
}

namespace slothjson
{
    // int8_t & uint8_t
    template<> inline bool slothjson_cxx::encode(const int8_t& obj_val, allocator_t& alloc, rapidjson::Value& json_val)
    {
        int32_t tmp_obj_val = static_cast<int32_t>(obj_val);
        return encode(tmp_obj_val, alloc, json_val);
    }
    template<> inline bool slothjson_cxx::decode(const rapidjson::Value& json_val, int8_t& obj_val)
    {
        int32_t tmp_obj_val = 0;
        if (!decode(json_val, tmp_obj_val))
        {
            return false;
        }
        obj_val = static_cast<int8_t>(tmp_obj_val);
        return true;
    }

    template<> inline bool slothjson_cxx::encode(const uint8_t& obj_val, allocator_t& alloc, rapidjson::Value& json_val)
    {
        uint32_t tmp_obj_val = static_cast<uint32_t>(obj_val);
        return encode(tmp_obj_val, alloc, json_val);
    }
    template<> inline bool slothjson_cxx::decode(const rapidjson::Value& json_val, uint8_t& obj_val)
    {
        uint32_t tmp_obj_val = 0;
        if (!decode(json_val, tmp_obj_val))
        {
            return false;
        }
        obj_val = static_cast<uint8_t>(tmp_obj_val);
        return true;
    }
    // int16_t & uint16_t
    template<> inline bool slothjson_cxx::encode(const int16_t& obj_val, allocator_t& alloc, rapidjson::Value& json_val)
    {
        int32_t tmp_obj_val = static_cast<int32_t>(obj_val);
        return encode(tmp_obj_val, alloc, json_val);
    }
    template<> inline bool slothjson_cxx::decode(const rapidjson::Value& json_val, int16_t& obj_val)
    {
        int32_t tmp_obj_val = 0;
        if (!decode(json_val, tmp_obj_val))
        {
            return false;
        }
        obj_val = static_cast<int16_t>(tmp_obj_val);
        return true;
    }

    template<> inline bool slothjson_cxx::encode(const uint16_t& obj_val, allocator_t& alloc, rapidjson::Value& json_val)
    {
        uint32_t tmp_obj_val = static_cast<uint32_t>(obj_val);
        return encode(tmp_obj_val, alloc, json_val);
    }
    template<> inline bool slothjson_cxx::decode(const rapidjson::Value& json_val, uint16_t& obj_val)
    {
        uint32_t tmp_obj_val = 0;
        if (!decode(json_val, tmp_obj_val))
        {
            return false;
        }
        obj_val = static_cast<uint16_t>(tmp_obj_val);
        return true;
    }

    // float
    template<> inline bool slothjson_cxx::encode(const float& obj_val, allocator_t& alloc, rapidjson::Value& json_val)
    {
        double tmp_obj_val = obj_val;
        return encode(tmp_obj_val, alloc, json_val);
    }
    template<> inline bool slothjson_cxx::decode(const rapidjson::Value& json_val, float & obj_val)
    {
        double tmp_obj_val = 0;
        if (!decode(json_val, tmp_obj_val))
        {
            return false;
        }
        obj_val = static_cast<float>(tmp_obj_val);
        return true;
    }

} // slothjson

namespace slothjson
{
    template <typename src_t, typename dst_t>
    bool slothjson_cxx::__convert(const src_t& src, dst_t& dst)
    {
        if (src.IsBool())
        {
            dst.SetBool(src.GetBool());
        }
        else if (src.IsInt())
        {
            dst.SetInt(src.GetInt());
        }
        else if (src.IsUint())
        {
            dst.SetUint(src.GetUint());
        }
        else if (src.IsInt64())
        {
            dst.SetInt64(src.GetInt64());
        }
        else if (src.IsUint64())
        {
            dst.SetUint64(src.GetUint64());
        }
        else if (src.IsDouble())
        {
            dst.SetDouble(src.GetDouble());
        }
        else if (src.IsString())
        {
            dst.SetString(rapidjson::StringRef(src.GetString()));
        }
        else if (src.IsNull())
        {
            dst.SetNull();
        }
        else
        {
            return false;
        }
        return true;
    }

    bool slothjson_cxx::encode_document(rapidjson::Value& src, rapidjson::Document& dst)
    {
        rapidjson::Document::AllocatorType& allocator = dst.GetAllocator();
        if (src.IsArray())
        {
            dst.SetArray();
            rapidjson::SizeType size = src.Size();
            for (rapidjson::SizeType i = 0; i < size; ++i)
            {
                dst.PushBack(src[i].Move(), allocator);
            }
            return true;
        }
        else if (src.IsObject())
        {
            dst.SetObject();
            for (rapidjson::Value::MemberIterator it = src.MemberBegin(); it != src.MemberEnd(); ++it)
            {
                dst.AddMember(rapidjson::StringRef(it->name.GetString()), it->value.Move(), allocator);
            }
            return true;
        }
        return __convert(src, dst);
    }
    bool slothjson_cxx::decode_document(rapidjson::Document& src, rapidjson::Value& dst)
    {
        rapidjson::Document::AllocatorType& allocator = src.GetAllocator();
        if (src.IsArray())
        {
            dst.SetArray();
            rapidjson::SizeType size = src.Size();
            for (rapidjson::SizeType i = 0; i < size; ++i)
            {
                dst.PushBack(src[i].Move(), allocator);
            }
            return true;
        }
        else if (src.IsObject())
        {
            dst.SetObject();
            for (rapidjson::Document::MemberIterator it = src.MemberBegin(); it != src.MemberEnd(); ++it)
            {
                dst.AddMember(rapidjson::StringRef(it->name.GetString()), it->value.Move(), allocator);
            }
            return true;
        }
        return __convert(src, dst);
    }
    char * json_loader_t::__load(const char * path)
    {
        if (!path)
        {
            return NULL;
        }
        FILE * f = fopen(path, "rb");
        if (!f)
        {
            return NULL;
        }
        fseek(f, 0, SEEK_END);
        long len = ftell(f);
        fseek(f, 0, SEEK_SET);
        char * data = (char *)malloc(len + 1);
        fread(data, 1, len, f);
        data[len] = '\0';
        fclose(f);
        return data;
    }

    json_loader_t::json_loader_t(const char * path)
    {
        data_ = __load(path);
    }
    json_loader_t::~json_loader_t()
    {
        if (data_)
        {
            free(data_);
            data_ = NULL;
        }
    }
    const char * json_loader_t::c_str()
    {
        return data_;
    }

}
#endif // __slothjson_hpp_h__
