#pragma once

#include "logging/LoggingAll.h"
#include "various/serializer/Serializer.h"

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <chrono>
#include <type_traits>

namespace l::serialization {

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

	extern const int32_t kHeaderIdentifier;

	struct HeaderValidity {

		friend zpp::serializer::access;
		template <typename Archive, typename Self>
		static void serialize(Archive& archive, Self& self) {
			archive(self.mIdentifier, self.mVersion);
		}

		bool Peek(std::vector<unsigned char>& data);
		bool IsIdentifierValid();
		bool IsVersionValid(int32_t latestVersion);

		int32_t mIdentifier;
		int32_t mVersion;
	};

	class SerializationBase : public zpp::serializer::polymorphic {
	public:
		using SaveArchive = zpp::serializer::archive<zpp::serializer::lazy_vector_memory_output_archive>;
		using LoadArchive = zpp::serializer::archive<zpp::serializer::memory_view_input_archive>;

		SerializationBase() :
			mIdentifier(0),
			mVersion(0),
			mLatestVersion(mVersion), 
			mUseIdentifier(false),
			mUseVersion(false),
			mUseFiletype(false)
		{}
		SerializationBase(int32_t minimumVersion, int32_t latestVersion, bool useVersion = true, bool useFiletype = false) :
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
		virtual ~SerializationBase() = default;

		SerializationBase& operator=(SerializationBase&& other) noexcept;
		SerializationBase& operator=(const SerializationBase& other) noexcept;
		SerializationBase(SerializationBase&& other) noexcept;
		SerializationBase(const SerializationBase& other) noexcept;

		void LoadArchiveData(std::vector<unsigned char>& data);
		void GetArchiveData(std::vector<unsigned char>& data);

		friend zpp::serializer::access;
		template <typename Archive, typename Self>
		static void serialize(Archive& archive, Self& self) {
			if constexpr (std::is_base_of<SaveArchive, Archive>{}) {
				int32_t* p = const_cast<int32_t*>(&self.mVersion);
				*p = self.mLatestVersion;

				if (self.mUseIdentifier) {
					archive(kHeaderIdentifier);
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
					ASSERT(fileIdentifier == kHeaderIdentifier);
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

		int32_t GetVersion() const;

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

		void UpgradeToLatest();
	};

	class Config : public SerializationBase {
	public:
		static const int32_t LatestVersion = 1;

		Config() : SerializationBase(0, LatestVersion), mName{}, mDefaultUserId{} {}
		Config(int32_t version, std::string name, std::string defaultUserId) : SerializationBase(version, LatestVersion), mName(name), mDefaultUserId(defaultUserId) {}
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

}
