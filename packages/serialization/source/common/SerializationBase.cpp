#include "serialization/SerializationBase.h"

#include "logging/LoggingAll.h"

#include <string>
#include <vector>
#include <array>
#include <iostream>
#include <ctime>

namespace l::serialization {
	const int32_t kHeaderIdentifier = 0x00defa00; // storage base file identifier

	const int32_t kTinyHeaderIdentifier = 0xdf000000; // tiny identifier
	const int32_t kTinyHeaderVersionMask = 0x00fff000;

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

	bool TinyHeaderValidity::Peek(std::vector<unsigned char>& data) {
		if (data.size() < 4) { // identifier + version = 8 bytes
			return false;
		}
		zpp::serializer::memory_input_archive_peek inPeek(data);
		inPeek(*this);
		return true;
	}

	bool TinyHeaderValidity::IsIdentifierValid() {
		return (mHeader & kTinyHeaderIdentifier) == kTinyHeaderIdentifier;
	}
	bool TinyHeaderValidity::IsVersionValid(int32_t minVersion, int32_t latestVersion) {
		auto version = (mHeader & kTinyHeaderVersionMask) >> 3;
		return version >= minVersion && version <= latestVersion;
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
		mUseTinyHeader = other.mUseTinyHeader;
		mExpectIdentifier = other.mExpectIdentifier;
		mExpectVersion = other.mExpectVersion;
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
		mUseTinyHeader = other.mUseTinyHeader;
		mExpectIdentifier = other.mExpectIdentifier;
		mExpectVersion = other.mExpectVersion;
		return *this;
	}
	SerializationBase::SerializationBase(SerializationBase&& other) noexcept {
		*this = std::move(other);
	}
	SerializationBase::SerializationBase(const SerializationBase& other) noexcept {
		*this = other;
	}

	void SerializationBase::LoadArchiveData(std::vector<unsigned char>& data) {
		// data should have identifier, but doesn't, or should not have identifier - don't load identifier (this is fine since we can determine if identifier is present or not)
		//   data should not have version - fine
		//   data should have version but doesn't - error (we can't know this)
		//   data should have version and seems to have version - load version (it is possible for false positives when checking for version in data so must be sure data contain version in this case)

		// data should have identifier, and does - load identifier
		//   no - (data should not have version - fine)
		//   no - (data should have version but doesn't - error (we can't know this)) - it must be there
		//   data should have version and does - load version

		if (mUseTinyHeader) {

		}
		else {
			HeaderValidity headerValidity;
			bool peekSuccessful = headerValidity.Peek(data);
			if (mExpectIdentifier) {
				if (!peekSuccessful || !headerValidity.IsIdentifierValid()) {
					LOG(LogError) << "Expected serialization identifier: " << headerValidity.mIdentifier << " (version: " << headerValidity.mVersion << ")";
					return;
				}
				if (!peekSuccessful || !headerValidity.IsVersionValid(mLatestVersion)) {
					LOG(LogError) << "Expected serialization version: " << headerValidity.mVersion << " (identifier: " << headerValidity.mIdentifier << ")";
					return;
				}
				mUseIdentifier = true; // if identifier is expected, we should continue using it even if user didn't not set it
				mExpectIdentifier = true;
				mExpectVersion = true;
				mUseVersion = true;
			}
			else if (peekSuccessful && headerValidity.IsIdentifierValid()) {
				mUseIdentifier = true;
				mExpectIdentifier = true;
				
				if (!headerValidity.IsVersionValid(mLatestVersion)) {
					LOG(LogError) << "Expected serialization version: " << headerValidity.mVersion;
					return;
				}

				mExpectVersion = true;
				mUseVersion = true;
			}
			else if (mExpectVersion) {
				if (!peekSuccessful || !headerValidity.IsVersionValid(mLatestVersion)) {
					return;
				}
				mUseVersion = true; // if version is expected, we should continue using it even if user didn't not set it
			}
			zpp::serializer::memory_input_archive in(data);
			in(*this);
		}
	}

	void SerializationBase::GetArchiveData(std::vector<unsigned char>& data) {
		zpp::serializer::memory_output_archive out(data);
		out(*this);
	}

	int32_t SerializationBase::GetVersion() const {
		return mVersion;
	}

	void SerializationBase::SaveHandler(SaveArchive& saveArchive) const {
		if (mUseTinyHeader) {

		}
		else {
			if (mUseIdentifier) {
				saveArchive(kHeaderIdentifier);
			}
			if (mUseVersion) {
				if (mVersion != mLatestVersion) { // make sure version is latest version 
					int32_t* p = const_cast<int32_t*>(&mVersion);
					*p = mLatestVersion;
				}
				saveArchive(mVersion);
			}
			if (mUseFiletype) {
				saveArchive(mFiletype);
			}
		}
		Save(saveArchive);
	}

	void SerializationBase::LoadHandler(LoadArchive& loadArchive) {
		if (mUseTinyHeader) {

		}
		else {
			if (mExpectIdentifier) {
				int32_t fileIdentifier;
				loadArchive(fileIdentifier);
				ASSERT(fileIdentifier == kHeaderIdentifier);
			}
			if (mExpectVersion) {
				loadArchive(mVersion);
			}
			if (mUseFiletype) {
				loadArchive(mFiletype);
			}
		}
		Load(loadArchive);
		UpgradeToLatest();
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
