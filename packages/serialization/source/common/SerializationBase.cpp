#include "serialization/SerializationBase.h"

#include "logging/LoggingAll.h"

#include <string>
#include <vector>
#include <array>
#include <iostream>
#include <ctime>

namespace l::serialization {
	const int32_t kHeaderIdentifier = 0x00defa00; // storage base file identifier

	bool HeaderValidity::Peek(std::vector<unsigned char>& data) {
		if (data.size() < 4 * 2) { // identifier + version = 8 bytes
			return false;
		}
		zpp::serializer::memory_input_archive_peek inPeek(data);
		inPeek(*this);
		return true;
	}

	bool HeaderValidity::IsIdentifierValid() {
		return mIdentifier == kHeaderIdentifier;
	}
	bool HeaderValidity::IsVersionValid(int32_t latestVersion) {
		if (mIdentifier != kHeaderIdentifier) {
			mVersion = mIdentifier; // we might have loaded data without identifier but with version, so check
		}
		return mVersion >= 0 && mVersion <= latestVersion;
	}

	/******************************************************************************/

	SerializationBase& SerializationBase::operator=(SerializationBase&& other) noexcept {
		mIdentifier = other.mIdentifier;
		mVersion = other.mVersion;
		mLatestVersion = other.mLatestVersion;
		mFiletype = other.mFiletype;
		mUseIdentifier = other.mUseIdentifier;
		mUseVersion = other.mUseVersion;
		mUseFiletype = other.mUseFiletype;
		return *this;
	}
	SerializationBase& SerializationBase::operator=(const SerializationBase& other) noexcept {
		mIdentifier = other.mIdentifier;
		mVersion = other.mVersion;
		mLatestVersion = other.mLatestVersion;
		mFiletype = other.mFiletype;
		mUseIdentifier = other.mUseIdentifier;
		mUseVersion = other.mUseVersion;
		mUseFiletype = other.mUseFiletype;
		return *this;
	}
	SerializationBase::SerializationBase(SerializationBase&& other) noexcept {
		*this = std::move(other);
	}
	SerializationBase::SerializationBase(const SerializationBase& other) noexcept {
		*this = other;
	}

	void SerializationBase::LoadArchiveData(std::vector<unsigned char>& data) {
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

	void SerializationBase::GetArchiveData(std::vector<unsigned char>& data) {
		zpp::serializer::memory_output_archive out(data);
		out(*this);
	}

	int32_t SerializationBase::GetVersion() const {
		return mVersion;
	}

	void SerializationBase::UpgradeToLatest() {
		mLatestVersion = mVersion > mLatestVersion ? mVersion : mLatestVersion;
		if (mUseVersion) {
			for (; mVersion < mLatestVersion; mVersion++) {
				Upgrade(mVersion);
			}
		}
		mVersion = mVersion < mLatestVersion ? mLatestVersion : mVersion;
	}


}
