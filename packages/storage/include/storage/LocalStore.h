#pragma once

#include "memory/Containers.h"
#include "serialization/SerializationBase.h"
#include "filesystem/File.h"

#include <string>
#include <vector>
#include <filesystem>
#include <memory>
#include <optional>
#include <chrono>
#include <fstream>
#include <type_traits>

namespace l {
namespace storage {

	bool read(const std::filesystem::path& mLocation, char* dst, size_t count, size_t position = 0);
	bool write(const std::filesystem::path& mLocation, const char* src, size_t count, size_t position = 0);

	bool read(const std::filesystem::path& mLocation, std::stringstream& data);
	bool write(const std::filesystem::path& mLocation, std::stringstream& data);

	class LocalStorage {
	public:
		LocalStorage(std::filesystem::path&& location);

		template<class T, class = meta::IsDerived<serialization::SerializationBase, T>>
		void Set(const std::string& key, std::unique_ptr<T> value) {
			mStore.emplace(key, std::move(value));
		}

		void Erase(const std::string& key) {
			mStore.erase(key);
		}

		template<class T>
		T* Get(const std::string& key) {
			auto it = mStore.find(key);
			if (it != mStore.end()) {
				return reinterpret_cast<T*>(it->second.get());
			}
			return nullptr;
		}

		template<class T, class = meta::IsDerived<serialization::SerializationBase, T>>
		bool Save(const std::string& key) {
			T* data = Get<T>(key);
			std::vector<unsigned char> bytes;

			data->GetArchiveData(bytes);

			auto name = mLocation.stem().string() + "_" + key;
			filesystem::File file(mLocation / name);

			file.modeBinary();
			file.modeWriteTrunc();

			if (!file.open()) {
				return false;
			}

			file.write(bytes.data(), bytes.size());
			file.close();

			return true;
		}

		template<class T, class = meta::IsDerived<serialization::SerializationBase, T>>
		bool Load(const std::string& key) {
			T* data = Get<T>(key);
			if (data == nullptr) {
				Set(key, std::make_unique<T>());
				data = Get<T>(key);
			}

			auto name = mLocation.stem().string() + "_" + key;
			filesystem::File file(mLocation / name);
			file.modeBinary();
			file.modeReadPreload();

			if (!file.open()) {
				return false;
			}


			std::vector<unsigned char> bytes;
			bytes.resize(file.fileSize());
			if (file.read(bytes.data(), bytes.size()) != bytes.size()) {
				return false;
			}

			file.close();

			data->LoadArchiveData(bytes);

			return true;
		}


	private:
		void CreatePath();

		std::filesystem::path mLocation;
		std::map<std::string, std::unique_ptr<serialization::SerializationBase>> mStore;
	};

}
}
