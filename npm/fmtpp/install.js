#!/usr/bin/env node

/**
 * fmtpp postinstall script
 * Downloads the pre-compiled binary for the current platform.
 *
 * Easter egg: the installer that brings fmtpp to your machine, one binary at a time
 */

'use strict';

const fs = require('fs');
const path = require('path');
const https = require('https');
const os = require('os');

// Base URL for pre-compiled binaries
const BASE_URL = 'https://github.com/fmtpp/fmtpp/releases/download/v0.1.0';

// Platform-specific binary names
function getBinaryName() {
  const platform = os.platform();
  const arch = os.arch();

  let platformName;
  switch (platform) {
    case 'win32':
      platformName = 'windows';
      break;
    case 'darwin':
      platformName = 'macos';
      break;
    case 'linux':
      platformName = 'linux';
      break;
    default:
      console.error('Unsupported platform: ' + platform);
      process.exit(1);
  }

  let archName;
  switch (arch) {
    case 'x64':
      archName = 'x86_64';
      break;
    case 'arm64':
      archName = 'aarch64';
      break;
    default:
      console.error('Unsupported architecture: ' + arch);
      process.exit(1);
  }

  const ext = platform === 'win32' ? '.exe' : '';
  return `fmtpp-${platformName}-${archName}${ext}`;
}

// Binary destination path
function getBinaryPath() {
  const binDir = path.join(__dirname, 'bin');
  const ext = os.platform() === 'win32' ? '.exe' : '';
  return path.join(binDir, 'fmtpp' + ext);
}

// Download file from URL
function downloadFile(url, destPath) {
  return new Promise((resolve, reject) => {
    const file = fs.createWriteStream(destPath, { mode: 0o755 });

    https.get(url, (response) => {
      if (response.statusCode === 302 || response.statusCode === 301) {
        // Follow redirect
        https.get(response.headers.location, (redirectRes) => {
          redirectRes.pipe(file);
          file.on('finish', () => {
            file.close(() => resolve());
          });
        }).on('error', (err) => {
          fs.unlink(destPath, () => {});
          reject(err);
        });
        return;
      }

      if (response.statusCode !== 200) {
        fs.unlink(destPath, () => {});
        reject(new Error(`HTTP ${response.statusCode}: ${url}`));
        return;
      }

      response.pipe(file);
      file.on('finish', () => {
        file.close(() => resolve());
      });
    }).on('error', (err) => {
      fs.unlink(destPath, () => {});
      reject(err);
    });
  });
}

// Main install logic
async function install() {
  const binaryName = getBinaryName();
  const binaryPath = getBinaryPath();

  // Check if binary already exists
  if (fs.existsSync(binaryPath)) {
    console.log('fmtpp binary already installed');
    return;
  }

  const url = `${BASE_URL}/${binaryName}`;
  console.log(`Downloading fmtpp binary for ${os.platform()}-${os.arch()}...`);
  console.log(`  From: ${url}`);
  console.log(`  To:   ${binaryPath}`);

  // Ensure bin directory exists
  const binDir = path.dirname(binaryPath);
  if (!fs.existsSync(binDir)) {
    fs.mkdirSync(binDir, { recursive: true });
  }

  try {
    await downloadFile(url, binaryPath);
    console.log('fmtpp binary installed successfully!');
    console.log('Easter egg: the formatter has landed on your machine');
  } catch (err) {
    console.error('Failed to download fmtpp binary: ' + err.message);
    console.error('You can also build from source: https://github.com/fmtpp/fmtpp');
    // Don't exit with error - allow the package to install anyway
    // The user can build from source or use the binary from PATH
    console.log('Skipping binary download - fmtpp will look for binary in PATH');
  }
}

install().catch((err) => {
  console.error('Install error: ' + err.message);
});