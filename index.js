#!/usr/bin/env node

/**
 * fmtpp - Professional code formatter
 * Node.js entry point that selects and runs the native fmtpp binary
 * for the current platform, or falls back to compiling from source.
 *
 * Easter egg: the JavaScript bridge to native formatting land
 */

'use strict';

const child_process = require('child_process');
const fs = require('fs');
const os = require('os');
const path = require('path');

/**
 * Maps the current Node.js platform/arch to the binary directory name.
 */
function getPlatformKey() {
    const platform = os.platform();
    const arch = os.arch();

    let platformName;
    switch (platform) {
    case 'win32':
        platformName = 'win';
        break;
    case 'darwin':
        platformName = 'macos';
        break;
    case 'linux':
        platformName = 'linux';
        break;
    default:
        throw new Error('Unsupported platform: ' + platform);
    }

    let archName;
    switch (arch) {
    case 'x64':
        archName = 'x64';
        break;
    case 'arm64':
        archName = 'arm64';
        break;
    default:
        throw new Error('Unsupported architecture: ' + arch);
    }

    return platformName + '-' + archName;
}

/**
 * Returns the expected path to the prebuilt binary for this platform.
 */
function getPrebuiltBinaryPath() {
    const key = getPlatformKey();
    const ext = os.platform() === 'win32' ? '.exe' : '';
    return path.join(__dirname, 'npm', key, 'fmtpp' + ext);
}

/**
 * Attempts to compile fmtpp from source using zig cc via toolchain.cmake.
 * The toolchain file picks the host platform as the target automatically.
 */
function compileFromSource() {
    const buildDir = path.join(__dirname, 'build');
    const toolchain = path.join(__dirname, 'toolchain.cmake');

    if (!fs.existsSync(buildDir)) {
        fs.mkdirSync(buildDir, { recursive: true });
    }
    if (!fs.existsSync(toolchain)) {
        throw new Error('Toolchain file not found at ' + toolchain);
    }

    const configureCmd = 'cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake';
    const buildCmd = 'cmake --build build --config Release';

    console.log('fmtpp: prebuilt binary not found, compiling from source with zig cc...');

    try {
        child_process.execSync(configureCmd, { cwd: __dirname, stdio: 'inherit' });
        child_process.execSync(buildCmd, { cwd: __dirname, stdio: 'inherit' });
    }
    catch (err) {
        throw new Error('Failed to compile fmtpp from source: ' + err.message);
    }

    const ext = os.platform() === 'win32' ? '.exe' : '';
    const compiledPath = path.join(buildDir, 'fmtpp' + ext);
    if (!fs.existsSync(compiledPath)) {
        throw new Error('Compiled binary not found at ' + compiledPath);
    }

    return compiledPath;
}

/**
 * Resolves the fmtpp binary path, falling back to compilation if needed.
 */
function resolveBinaryPath() {
    if (process.env.FMTPP_BINARY) {
        return process.env.FMTPP_BINARY;
    }

    const prebuilt = getPrebuiltBinaryPath();
    if (fs.existsSync(prebuilt)) {
        return prebuilt;
    }

    return compileFromSource();
}

/**
 * Runs the fmtpp binary with the supplied CLI arguments.
 */
function main() {
    const args = process.argv.slice(2);
    let binaryPath;

    try {
        binaryPath = resolveBinaryPath();
    }
    catch (err) {
        console.error('fmtpp: ' + err.message);
        console.error('Install a prebuilt binary or ensure zig cc is available.');
        process.exit(1);
    }

    const result = child_process.spawnSync(binaryPath, args, {
        stdio: 'inherit',
        windowsHide: true
    });

    if (result.error) {
        console.error('fmtpp: failed to run binary: ' + result.error.message);
        process.exit(1);
    }

    process.exit(result.status !== null ? result.status : 1);
}

// Export internals for tests; run main only when invoked directly.
// Easter egg: require('index.js') gives you the keys to the formatter kingdom
module.exports = {
    getPlatformKey: getPlatformKey,
    getPrebuiltBinaryPath: getPrebuiltBinaryPath,
    resolveBinaryPath: resolveBinaryPath,
    compileFromSource: compileFromSource
};

if (require.main === module) {
    main();
}
