import os
import re
import glob
import argparse

def parse_cmake(file_path):
    version_dict = {}
    found_major = found_minor = found_build = False

    with open(file_path, 'r') as file:
        for line in file:
            if not found_major and "MATERIALX_MAJOR_VERSION" in line:
                major_match = re.search(r"set\(MATERIALX_MAJOR_VERSION (\d+)\)", line)
                if major_match:
                    version_dict['MATERIALX_MAJOR_VERSION'] = major_match.group(1)
                    found_major = True
            elif not found_minor and "MATERIALX_MINOR_VERSION" in line:
                minor_match = re.search(r"set\(MATERIALX_MINOR_VERSION (\d+)\)", line)
                if minor_match:
                    version_dict['MATERIALX_MINOR_VERSION'] = minor_match.group(1)
                    found_minor = True
            elif not found_build and "MATERIALX_BUILD_VERSION" in line:
                build_match = re.search(r"set\(MATERIALX_BUILD_VERSION (\d+)\)", line)
                if build_match:
                    version_dict['MATERIALX_BUILD_VERSION'] = build_match.group(1)
                    found_build = True

            # If all components have been found, break out of the loop
            if found_major and found_minor and found_build:
                break
    return version_dict

def update_version_properties(file_path, version_info, patch_version):
    version_short = f"{version_info['MATERIALX_MAJOR_VERSION']}.{version_info['MATERIALX_MINOR_VERSION']}{version_info['MATERIALX_BUILD_VERSION']}.{patch_version}"
    version_full = f"{version_info['MATERIALX_MAJOR_VERSION']}.{version_info['MATERIALX_MINOR_VERSION']}.{version_info['MATERIALX_BUILD_VERSION']}"

    with open(file_path, 'r') as file:
        content = file.read()
    
    content_new = re.sub(r"version=\d+\.\d+\.\d+", f"version={version_short}", content)
    content_new = re.sub(r"materialx_version=\d+\.\d+\.\d+", f"materialx_version={version_full}", content_new)

    with open(file_path, 'w') as file:
        file.write(content_new)
        print(f"Updated version in file: {file_path}")

def update_version_rc(file_path, version_info, patch_version):
    with open(file_path, 'r') as file:
        content = file.read()
    
    content = re.sub(r'FILEVERSION\s+\d+,\d+,\d+,\d+', f"FILEVERSION    {version_info['MATERIALX_MAJOR_VERSION']},{version_info['MATERIALX_MINOR_VERSION']},{version_info['MATERIALX_BUILD_VERSION']},{patch_version}", content)
    content = re.sub(r'PRODUCTVERSION\s+\d+,\d+,\d+,\d+', f"PRODUCTVERSION {version_info['MATERIALX_MAJOR_VERSION']},{version_info['MATERIALX_MINOR_VERSION']},{version_info['MATERIALX_BUILD_VERSION']},0", content)
    content = re.sub(r'VALUE "FileVersion",\s+"\d+\.\d+\.\d+\.\d+', f"VALUE \"FileVersion\",        \"{version_info['MATERIALX_MAJOR_VERSION']}.{version_info['MATERIALX_MINOR_VERSION']}.{version_info['MATERIALX_BUILD_VERSION']}.{patch_version}", content)
    content = re.sub(r'VALUE "ProductVersion",\s+"\d+\.\d+\.\d+\.\d+', f"VALUE \"ProductVersion\",     \"{version_info['MATERIALX_MAJOR_VERSION']}.{version_info['MATERIALX_MINOR_VERSION']}.{version_info['MATERIALX_BUILD_VERSION']}.0", content)

    with open(file_path, 'w') as file:
        file.write(content)
        print(f"Updated version in file: {file_path}")

def discover_and_update_files(directory, version_info, patch_version):
    properties_files = glob.glob(os.path.join(directory, '*.properties'))
    rc_files = glob.glob(os.path.join(directory, '*.rc'))
    
    for properties_file in properties_files:
        update_version_properties(properties_file, version_info, patch_version)

    for rc_file in rc_files:
        update_version_rc(rc_file, version_info, patch_version)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Update version information in .properties and .rc files.')
    parser.add_argument('--cmake', required=True, help='Path to CMakeLists.txt file.')
    parser.add_argument('--directory', required=True, help='Directory path where .properties and .rc files are located.')
    parser.add_argument('--patch', default='0', help='Patch version number (default: %(default)s).', type=str)

    args = parser.parse_args()

    cmake_file_path = args.cmake
    directory_to_search = args.directory
    patch_version = args.patch

    version_info = parse_cmake(cmake_file_path)
    discover_and_update_files(directory_to_search, version_info, patch_version)

    print(f"Updated versions in all .properties and .rc files in the directory.")
