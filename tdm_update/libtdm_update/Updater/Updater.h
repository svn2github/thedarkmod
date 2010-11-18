#pragma once

#include <stdexcept>
#include "UpdaterOptions.h"
#include "../Http/HttpConnection.h"
#include "../Http/MirrorList.h"
#include "../Http/DownloadManager.h"
#include "../ReleaseFileset.h"
#include "../ReleaseVersions.h"
#include "../UpdatePackageInfo.h"
#include "DifferentialUpdateInfo.h"

/**
 * Main updater class containing the application logic. 
 * Use an instance of this class to control the update process 
 * by calling the appropriate methods, like UpdateMirrors().
 */
namespace tdm
{

namespace updater
{

struct CurDownloadInfo
{
	// The filename we're downloading
	fs::path file;

	// The progress fraction
	double progressFraction;

	// In bytes/sec
	double downloadSpeed;

	// Number of bytes received
	std::size_t downloadedBytes;

	// The display name of the currently active mirror
	std::string mirrorDisplayName;
};

struct CurFileInfo
{
	enum Operation
	{
		Check,
		Delete,
		Replace,
		Add,
		RemoveFilesFromPK4,
	};

	Operation operation;

	// The file which is being worked on
	fs::path file;

	// The progress of the whole operation
	double overallProgressFraction;
};

class Updater
{
public:
	// Thrown on any errors during method execution
	class FailureException :
		public std::runtime_error
	{
	public:
		FailureException(const std::string& message) :
			std::runtime_error(message)
		{}
	};

	// An object to get notified in regular intervals about the download progress
	class DownloadProgress
	{
	public:
		virtual void OnDownloadProgress(const CurDownloadInfo& info) = 0;

		virtual void OnDownloadFinish() = 0;
	};
	typedef boost::shared_ptr<DownloadProgress> DownloadProgressPtr;

	// An object to get notified in regular intervals about the file operations
	class FileOperationProgress
	{
	public:
		virtual void OnFileOperationProgress(const CurFileInfo& info) = 0;

		virtual void OnFileOperationFinish() = 0;
	};
	typedef boost::shared_ptr<FileOperationProgress> FileOperationProgressPtr;

private:
	const UpdaterOptions& _options;

	// The HTTP connection object
	HttpConnectionPtr _conn;

	// The mirror information
	MirrorList _mirrors;

	DownloadManagerPtr _downloadManager;

	// The latest release file set on the server
	ReleaseFileSet _latestRelease;

	// The release files which should be downloaded
	ReleaseFileSet _downloadQueue;

	// Some files like DoomConfig.cfg or dmargs.txt are ignored.
	std::set<std::string> _ignoreList;

	DownloadProgressPtr _downloadProgressCallback;

	FileOperationProgressPtr _fileProgressCallback;

	// The name of the updater executable, to detect self-updates
	fs::path _executable;

	// True if we've downloaded an update for the tdm_update binary
	bool _updatingUpdater;

#ifdef WIN32
	// The name of the Win32 update batch file (not needed for *nix builds)
	fs::path _updateBatchFile;
#endif

	// The version information, indexed by version string "1.02" => [ReleaseFileSet]
	ReleaseVersions _releaseVersions;

	UpdatePackageInfo _updatePackages;

	// The version the local installation has been found to match (empty if no match)
	std::string _localVersion;

public:
	// Pass the program options to this class
	Updater(const UpdaterOptions& options, const fs::path& executable);

	// Removes leftovers from previous run
	void CleanupPreviousSession();

	// Returns TRUE if new mirrors should be downloaded
	bool MirrorsNeedUpdate();

	// Returns the number of registered mirrors
	std::size_t GetNumMirrors();

	// Update the local tdm_mirrors.txt file from the main servers.
	void UpdateMirrors();

	// Load information from the tdm_mirrors.txt file
	void LoadMirrors();

	// Download the checksum crc_info.txt file from the mirrors.
	void GetCrcFromServer();

	// Download the tdm_version_info.txt file from a mirror.
	void GetVersionInfoFromServer();

	// Return the version string of the newest available version
	std::string GetNewestVersion();

	// Attempt to determine the local version using the version info downloaded earlier
	void DetermineLocalVersion();

	// Returns true if a differential update is available (based on this version)
	bool DifferentialUpdateAvailable();

	// Returns the total size of all differential updates leading to the newest version
	std::size_t GetTotalDifferentialUpdateSize();

	// Downloads the package applicable to the local version
	void DownloadDifferentialUpdate();

	// Apply changes to the local installation
	void PerformDifferentialUpdateStep();

	// Return the version as determined in DetermineLocalVersion()
	std::string GetDeterminedLocalVersion();

	// Get Information about the next differential update
	DifferentialUpdateInfo GetDifferentialUpdateInfo();

	// Compare locally installed files to the "ideal" state defined in crc_info.txt 
	void CheckLocalFiles();

	// True if the local files are not up to date and downloads need to be started
	bool LocalFilesNeedUpdate();

	// Generates an internal TODO list needed to perform the update step
	void PrepareUpdateStep();

	// Performs the update step, downloads stuff and integrates the package
	void PerformUpdateStep();

	// Performs any post-update step actions, if any
	void CleanupUpdateStep();

	std::size_t GetNumFilesToBeUpdated();

	// Returns the number of bytes which need to be downloaded
	std::size_t GetTotalDownloadSize();

	// Calls with information about the current download status (periodically)
	void SetDownloadProgressCallback(const DownloadProgressPtr& callback);
	void ClearDownloadProgressCallback();

	// Calls with information about the current file operation status (periodically)
	void SetFileOperationProgressCallback(const FileOperationProgressPtr& callback);
	void ClearFileOperationProgressCallback();

	// Returns TRUE if the tdm_update binary needs an update
	bool NewUpdaterAvailable();

	// Removes all packages, except the one containing the tdm_update binary
	void RemoveAllPackagesExceptUpdater();

	// Returns true if an updater update is pending and a restart is necessary
	bool RestartRequired();

	// Re-launches the updater (starts update batch file in Win32 builds)
	void RestartUpdater();

	// Cleanup (both after regular exit and user terminations)
	void PostUpdateCleanup();

	// Interrupts ongoing downloads
	void CancelDownloads();

private:
	// Throws if mirrors are empty
	void AssertMirrorsNotEmpty();

	void NotifyDownloadProgress();

	// Notifier shortcut
	void NotifyFileProgress(const fs::path& file, CurFileInfo::Operation op, double fraction);

	// Returns false if the local files is missing or needs an update
	bool CheckLocalFile(const fs::path& installPath, const ReleaseFile& releaseFile);

	// Get the target path (defaults to current path)
	fs::path GetTargetPath();

	// Extract the contents of the given zip file (and remove the zip afterwards)
	void ExtractAndRemoveZip(const fs::path& zipFilePath);

	// Downloads a file from a random mirror to the target folder
	void PerformSingleMirroredDownload(const std::string& remoteFile);

	// Checks if the given update package is already present at the given location
	bool VerifyUpdatePackageAt(const UpdatePackage& info, const fs::path& package);

#ifdef WIN32
	// Prepare the update batch file (for Win32 builds only)
	void PrepareUpdateBatchFile(const fs::path& temporaryUpdater);
#endif
};

} // namespace

} // namespace

