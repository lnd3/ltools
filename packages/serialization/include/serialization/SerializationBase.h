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


	extern const int32_t kHeaderIdentifier;
	extern const int32_t kTinyHeaderIdentifier;

	struct HeaderValidity {
		friend zpp::serializer::access;
		template <typename Archive, typename Self>
		static void serialize(Archive& archive, Self& self) {
			archive(self.mIdentifier, self.mVersion);
		}

		bool Peek(std::vector<unsigned char>& data);
		bool IsIdentifierValid();
		bool IsVersionValid(int32_t latestVersion);

		int32_t mIdentifier = 0;
		int32_t mVersion = 0;
	};

	struct TinyHeaderValidity {
		friend zpp::serializer::access;
		template <typename Archive, typename Self>
		static void serialize(Archive& archive, Self& self) {
			archive(self.mHeader);
		}

		bool Peek(std::vector<unsigned char>& data);
		bool IsIdentifierValid();
		bool IsVersionValid(int32_t minVersion, int32_t latestVersion);

		int32_t mHeader;
	};

	class SerializationBase : public zpp::serializer::polymorphic {
	public:
		using SaveArchive = zpp::serializer::archive<zpp::serializer::lazy_vector_memory_output_archive>;
		using LoadArchive = zpp::serializer::archive<zpp::serializer::memory_view_input_archive>;

		// default creator for loading of existing data and should be able to handle all cases
		SerializationBase() :
			mIdentifier(0),
			mVersion(0),
			mLatestVersion(mVersion), 
			mUseIdentifier(false),
			mUseVersion(false),
			mUseFiletype(false),
			mUseTinyHeader(false),
			mExpectIdentifier(false),
			mExpectVersion(false)
		{}
		SerializationBase(int32_t minimumVersion, int32_t latestVersion, bool useVersion = true, bool useFiletype = false, bool useIdentifier = false, bool expectIdentifier = false, bool expectVersion = false, bool useTinyHeader = false) :
			mIdentifier(0),
			mVersion(minimumVersion),
			mLatestVersion(latestVersion),
			mUseIdentifier(useIdentifier),
			mUseVersion(useVersion || useTinyHeader),
			mUseFiletype(useFiletype),
			mUseTinyHeader(useTinyHeader),
			mExpectIdentifier(expectIdentifier),
			mExpectVersion(expectVersion)
		{
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
				auto& saveArchive = *reinterpret_cast<SaveArchive*>(&archive);
				self.SaveHandler(saveArchive);
			}
			if constexpr (std::is_base_of<LoadArchive, Archive>{}) {
				auto& loadArchive = *reinterpret_cast<LoadArchive*>(&archive);
				self.LoadHandler(loadArchive);
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

		bool mUseTinyHeader = false;

		bool mExpectIdentifier = false;
		bool mExpectVersion = false;

		void SaveHandler(SaveArchive& saveArchive) const;
		void LoadHandler(LoadArchive& loadArchive);
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
