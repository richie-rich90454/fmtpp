#!/usr/bin/env node

/**
 * fmtpp - Professional code formatter
 * Node.js wrapper that invokes the native fmtpp binary.
 *
 * Easter egg: the JavaScript bridge to native formatting land
 */

'use strict';

const path = require('path');
const child_process = require('child_process');
const fs = require('fs');
const os = require('os');

/**
 * Resolves the fmtpp binary path.
 * Checks in order:
 * 1. FMTpp_BINARY environment variable
 * 2. Bundled binary in npm package
 * 3. fmtpp in system PATH
 */
function getBinaryPath() {
  // Check environment variable
  if (process.env.FMTpp_BINARY) {
    return process.env.FMTpp_BINARY;
  }

  // Check bundled binary
  const ext = os.platform() === 'win32' ? '.exe' : '';
  const bundledPath = path.join(__dirname, 'bin', 'fmtpp' + ext);
  if (fs.existsSync(bundledPath)) {
    return bundledPath;
  }

  // Fall back to PATH
  return 'fmtpp';
}

/**
 * Runs fmtpp with the given arguments.
 */
function runFmtpp(args) {
  const binaryPath = getBinaryPath();
  const options = {
    stdio: 'inherit',
    windowsHide: true
  };

  try {
    const result = child_process.spawnSync(binaryPath, args, options);
    if (result.error) {
      if (result.error.code === 'ENOENT') {
        console.error('fmtpp binary not found.');
        console.error('Install it with: npm install -g fmtpp');
        console.error('Or build from source: https://github.com/fmtpp/fmtpp');
        process.exit(1);
      }
      throw result.error;
    }
    process.exit(result.status);
  } catch (err) {
    console.error('Failed to run fmtpp: ' + err.message);
    process.exit(1);
  }
}

// Run fmtpp with all arguments passed to this script
const args = process.argv.slice(2);
runFmtpp(args);