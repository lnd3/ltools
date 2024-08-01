#pragma once

#include "memory/Containers.h"
#include "various/serializer/Serializer.h"
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

	template<class T>
	void convert(std::stringstream& dst, std::vector<T>& src, size_t count = 0) {
		static_assert(sizeof(T) == sizeof(char));
		dst.write(reinterpret_cast<char*>(src.data()), count > 0 ? count : src.size());
	}

	template<class T, const size_t SIZE>
	void convert(std::stringstream& dst, std::array<T, SIZE>& src, size_t count = 0) {
		static_assert(sizeof(T) == sizeof(char));
		dst.write(reinterpret_cast<char*>(src.data()), count > 0 ? count : src.size());
	}

	template<class T>
	void convert(std::vector<T>& dst, std::stringstream& src) {
		T tmp{};
		static_assert(sizeof(T) == sizeof(char));
		while (src >> tmp) dst.push_back(tmp);
	}

	bool read(const std::filesystem::path& mLocation, char* dst, size_t count, size_t position = 0);
	bool write(const std::filesystem::path& mLocation, const char* src, size_t count, size_t position = 0);

	bool read(const std::filesystem::path& mLocation, std::stringstream& data);
	bool write(const std::filesystem::path& mLocation, std::stringstream& data);

	struct HeaderValidity {
		static const int32_t kIdentifier = 0x00defa00; // storage base file identifier

		friend zpp::serializer::access;
		template <typename Archive, typename Self>
		static void serialize(Archive& archive, Self& self) {
			archive(self.mIdentifier, self.mVersion);
		}

		bool Peek(std::vector<unsigned char>& data) {
			if (data.size() < 4*2) { // identifier + version = 8 bytes
				return false;
			}
			zpp::serializer::memory_input_archive_peek inPeek(data);
			inPeek(*this);
			return true;
		}

		bool IsIdentifierValid() {
			return mIdentifier == kIdentifier;
		}
		bool IsVersionValid(int32_t latestVersion) {
			if (mIdentifier != kIdentifier) {
				mVersion = mIdentifier; // we might have loaded data without identifier but with version, so check
			}
			return mVersion >= 0 && mVersion <= latestVersion;
		}

		int32_t mIdentifier;
		int32_t mVersion;
	};

	class StorageBase : public zpp::serializer::polymorphic {
	public:
		using SaveArchive = zpp::serializer::archive<zpp::serializer::lazy_vector_memory_output_archive>;
		using LoadArchive = zpp::serializer::archive<zpp::serializer::memory_view_input_archive>;

		StorageBase() : 
			mIdentifier(0),
			mVersion(0),
			mLatestVersion(mVersion), 
			mUseIdentifier(false),
			mUseVersion(false),
			mUseFiletype(false)
		{}
		StorageBase(int32_t minimumVersion, int32_t latestVersion, bool useVersion = true, bool useFiletype = false) : 
			mIdentifier(0),
			mVersion(minimumVersion),
			mLatestVersion(latestVersion),
			mUseIdentifier(false),
			mUseVersion(useVersion),
			mUseFiletype(useFiletype) {
			if (useVersion) {
				ASSERT(minimumVersion <= mLatestVersion);
			}
		}
		virtual ~StorageBase() = default;

		StorageBase& operator=(StorageBase&& other) noexcept {
			mIdentifier = other.mIdentifier;
			mVersion = other.mVersion;
			mLatestVersion = other.mLatestVersion;
			mFiletype = other.mFiletype;
			mUseIdentifier = other.mUseIdentifier;
			mUseVersion = other.mUseVersion;
			mUseFiletype = other.mUseFiletype;
			return *this;
		}
		StorageBase& operator=(const StorageBase& other) noexcept {
			mIdentifier = other.mIdentifier;
			mVersion = other.mVersion;
			mLatestVersion = other.mLatestVersion;
			mFiletype = other.mFiletype;
			mUseIdentifier = other.mUseIdentifier;
			mUseVersion = other.mUseVersion;
			mUseFiletype = other.mUseFiletype;
			return *this;
		}
		StorageBase(StorageBase&& other) noexcept {
			*this = std::move(other);
		}
		StorageBase(const StorageBase& other) noexcept {
			*this = other;
		}

		void LoadArchiveData(std::vector<unsigned char>& data) {
			HeaderValidity headerValidity;
			bool peekSuccessful = headerValidity.Peek(data);
			if (peekSuccessful && headerValidity.IsIdentifierValid()) {
				mUseIdentifier = true;
				mUseVersion = true;
			}
			else {
				mUseIdentifier = false;
				if (mUseVersion) {
					ASSERT(peekSuccessful && headerValidity.IsVersionValid(mLatestVersion));
				}
			}

			zpp::serializer::memory_input_archive in(data);
			in(*this);

			mUseIdentifier = true;
			mUseVersion = true;
		}

		void GetArchiveData(std::vector<unsigned char>& data) {
			zpp::serializer::memory_output_archive out(data);
			out(*this);
		}

		friend zpp::serializer::access;
		template <typename Archive, typename Self>
		static void serialize(Archive& archive, Self& self) {
			if constexpr (std::is_base_of<SaveArchive, Archive>{}) {
				int32_t* p = const_cast<int32_t*>(&self.mVersion);
				*p = self.mLatestVersion;

				if (self.mUseIdentifier) {
					archive(HeaderValidity::kIdentifier);
				}
				if (self.mUseVersion) {
					archive(self.mVersion);
				}
				if (self.mUseFiletype) {
					archive(self.mFiletype);
				}
				self.Save(archive);
			}
			if constexpr (std::is_base_of<LoadArchive, Archive>{}) {
				if (self.mUseIdentifier) {
					int32_t fileIdentifier;
					archive(fileIdentifier);
					ASSERT(fileIdentifier == HeaderValidity::kIdentifier);
				}
				if (self.mUseVersion) {
					archive(self.mVersion);
				}
				if (self.mUseFiletype) {
					archive(self.mFiletype);
				}
				self.Load(archive);
				self.UpgradeToLatest();
			}
		}

		int32_t GetVersion() const {
			return mVersion;
		}

	protected:
		virtual void Save(SaveArchive&) const {}
		virtual void Load(LoadArchive&) {}
		virtual void Upgrade(int32_t) {}

		int32_t mIdentifier = 0;
		int32_t mVersion = 0;
		std::string mFiletype;

		int32_t mLatestVersion = 0;
		bool mUseIdentifier = true;
		bool mUseVersion = true;
		bool mUseFiletype = false;

		void UpgradeToLatest() {
			mLatestVersion = mVersion > mLatestVersion ? mVersion : mLatestVersion;
			if (mUseVersion) {
				for (; mVersion < mLatestVersion; mVersion++) {
					Upgrade(mVersion);
				}
			}
			mVersion = mVersion < mLatestVersion ? mLatestVersion : mVersion;
		}
	};

	class Config : public StorageBase {
	public:
		static const int32_t LatestVersion = 1;

		Config() : StorageBase(0, LatestVersion), mName{}, mDefaultUserId{} {}
		Config(int32_t version, std::string name, std::string defaultUserId) : StorageBase(version, LatestVersion), mName(name), mDefaultUserId(defaultUserId) {}
		virtual ~Config() = default;

		friend zpp::serializer::access;
		virtual void Save(SaveArchive& archive) const {
			archive(mName, mDefaultUserId);
		}
		virtual void Load(LoadArchive& archive) {
			archive(mName, mDefaultUserId);
		}

		virtual void Upgrade(int32_t) {

		}

		std::string mName;
		std::string mDefaultUserId;
	};

	class LocalStorage {
	public:
		LocalStorage(std::filesystem::path&& location);

		template<class T, class = meta::IsDerived<StorageBase, T>>
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

		template<class T, class = meta::IsDerived<StorageBase, T>>
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

		template<class T, class = meta::IsDerived<StorageBase, T>>
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
		std::map<std::string, std::unique_ptr<StorageBase>> mStore;
	};

}
}
