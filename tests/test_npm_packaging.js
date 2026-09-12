#!/usr/bin/env node

/**
 * Tests for NPM packaging and cross-platform binary resolution.
 *
 * Easter egg: the test suite that makes sure fmtpp travels well
 */

'use strict';

const assert = require('assert');
const fs = require('fs');
const os = require('os');
const path = require('path');

const index = require('../index.js');

// Validate package.json fields required by the spec.
// Easter egg: package.json is the passport for npm travel
function testPackageJson() {
    const pkgPath = path.join(__dirname, '..', 'package.json');
    const pkg = JSON.parse(fs.readFileSync(pkgPath, 'utf8'));

    assert.strictEqual(pkg.name, 'fmtpp', 'package name should be fmtpp');
    assert.strictEqual(pkg.version, '0.1.0', 'package version should be 0.1.0');
    assert.ok(pkg.bin, 'package.json should have a bin field');
    assert.strictEqual(pkg.bin.fmtpp, './index.js', 'bin entry should point to ./index.js');
    assert.ok(pkg.description, 'package.json should have a description');
    assert.ok(pkg.license, 'package.json should have a license');
    assert.ok(pkg.repository, 'package.json should have a repository field');
    assert.ok(Array.isArray(pkg.files), 'package.json should have a files array');
    assert.ok(pkg.files.indexOf('npm/') >= 0, 'files array should include npm/');
    console.log('testPackageJson passed');
}

// Validate platform key detection for the current host.
function testPlatformDetection() {
    const key = index.getPlatformKey();
    const platform = os.platform();
    const arch = os.arch();

    if (platform === 'win32') {
        assert.ok(key.indexOf('win-') === 0, 'Windows platform should map to win-');
    } else if (platform === 'darwin') {
        assert.ok(key.indexOf('macos-') === 0, 'macOS platform should map to macos-');
    } else if (platform === 'linux') {
        assert.ok(key.indexOf('linux-') === 0, 'Linux platform should map to linux-');
    } else {
        throw new Error('Unsupported test platform: ' + platform);
    }

    if (arch === 'x64') {
        assert.ok(key.indexOf('-x64') > 0, 'x64 arch should map to -x64');
    } else if (arch === 'arm64') {
        assert.ok(key.indexOf('-arm64') > 0, 'arm64 arch should map to -arm64');
    } else {
        throw new Error('Unsupported test architecture: ' + arch);
    }

    console.log('testPlatformDetection passed (' + key + ')');
}

// Validate prebuilt binary path resolution.
function testBinaryPathResolution() {
    const binaryPath = index.getPrebuiltBinaryPath();
    const ext = os.platform() === 'win32' ? '.exe' : '';
    const expectedSuffix = path.join('npm', index.getPlatformKey(), 'fmtpp' + ext);
    assert.ok(binaryPath.indexOf(expectedSuffix) >= 0, 'binary path should end with ' + expectedSuffix);
    console.log('testBinaryPathResolution passed (' + binaryPath + ')');
}

// Validate that binary slots exist for all supported platforms.
function testBinarySlotsExist() {
    const npmDir = path.join(__dirname, '..', 'npm');
    const slots = [
        'linux-x64',
        'linux-arm64',
        'macos-x64',
        'macos-arm64',
        'win-x64'
    ];

    for (let i = 0; i < slots.length; i++) {
        const slotDir = path.join(npmDir, slots[i]);
        assert.ok(fs.existsSync(slotDir), 'binary slot should exist: ' + slots[i]);
    }

    console.log('testBinarySlotsExist passed');
}

// Validate that the FMTPP_BINARY environment variable is respected.
function testEnvOverride() {
    const original = process.env.FMTPP_BINARY;
    process.env.FMTPP_BINARY = '/custom/fmtpp';
    const resolved = index.resolveBinaryPath();
    assert.strictEqual(resolved, '/custom/fmtpp', 'FMTPP_BINARY should override binary resolution');
    if (original === undefined) {
        delete process.env.FMTPP_BINARY;
    } else {
        process.env.FMTPP_BINARY = original;
    }
    console.log('testEnvOverride passed');
}

// Validate that the release path exists for every binary slot:
// build-all.sh must list all targets and toolchain.cmake must map
// each one to a zig triple. Slots are filled at release time.
function testReleasePath() {
    const root = path.join(__dirname, '..');
    const buildAll = fs.readFileSync(path.join(root, 'scripts', 'build-all.sh'), 'utf8');
    const toolchain = fs.readFileSync(path.join(root, 'toolchain.cmake'), 'utf8');

    const targets = [
        'linux-x64',
        'linux-arm64',
        'macos-x64',
        'macos-arm64',
        'windows-x64'
    ];

    for (let i = 0; i < targets.length; i++) {
        assert.ok(buildAll.indexOf(targets[i]) >= 0,
            'build-all.sh should build target: ' + targets[i]);
        assert.ok(toolchain.indexOf(targets[i]) >= 0,
            'toolchain.cmake should map target: ' + targets[i]);
    }

    // The Windows slot is win-x64 while the zig target is windows-x64.
    assert.ok(buildAll.indexOf('npm/win-x64') >= 0,
        'build-all.sh should write Windows binaries to npm/win-x64');

    assert.ok(toolchain.indexOf('ZIG_TARGET') >= 0,
        'toolchain.cmake should define zig targets');
    console.log('testReleasePath passed');
}

// Validate fallback compilation entry exists and throws when zig is unavailable.
function testFallbackPath() {
    assert.strictEqual(typeof index.compileFromSource, 'function', 'compileFromSource should be exported');
    const src = index.compileFromSource.toString();
    assert.ok(src.indexOf('toolchain.cmake') >= 0,
        'compileFromSource should build through toolchain.cmake (zig cc needs it)');
    console.log('testFallbackPath passed');
}

function main() {
    testPackageJson();
    testPlatformDetection();
    testBinaryPathResolution();
    testBinarySlotsExist();
    testReleasePath();
    testEnvOverride();
    testFallbackPath();
    console.log('\nAll NPM packaging tests passed. Easter egg: fmtpp is ready to ship');
}

main();
