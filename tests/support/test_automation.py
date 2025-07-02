#!/usr/bin/env python3
"""
CGame Test Discovery and Automation

This script provides automated test discovery, execution, and coverage analysis
for the CGame engine test suite.
"""

import os
import sys
import subprocess
import json
import time
from pathlib import Path
from typing import List, Dict, Tuple
import argparse

class TestDiscovery:
    """Discovers and categorizes tests based on directory structure"""
    
    def __init__(self, test_root: str = "tests"):
        self.test_root = Path(test_root)
        self.categories = {
            'unit': ['core', 'systems', 'rendering', 'assets', 'ui', 'scenes'],
            'integration': ['integration'],
            'performance': ['performance'],
            'regression': ['regression'],
            'backlog': ['backlog']
        }
    
    def discover_tests(self) -> Dict[str, List[str]]:
        """Discover all test files organized by category"""
        discovered = {cat: [] for cat in self.categories.keys()}
        
        for category, directories in self.categories.items():
            for directory in directories:
                test_dir = self.test_root / directory
                if test_dir.exists():
                    for test_file in test_dir.rglob("test_*.c"):
                        discovered[category].append(str(test_file))
        
        return discovered
    
    def get_test_metadata(self, test_file: str) -> Dict:
        """Extract metadata from test file headers"""
        metadata = {
            'file': test_file,
            'description': '',
            'dependencies': [],
            'sprint': None,
            'critical': False
        }
        
        try:
            with open(test_file, 'r') as f:
                content = f.read(1000)  # Read first 1000 chars for metadata
                
                # Extract description from file header
                if '* @brief' in content:
                    start = content.find('* @brief') + 8
                    end = content.find('\n', start)
                    if end > start:
                        metadata['description'] = content[start:end].strip()
                
                # Check for critical tests
                if 'CRITICAL' in content.upper() or 'critical' in test_file:
                    metadata['critical'] = True
                
                # Extract sprint information
                if 'Sprint' in content:
                    import re
                    sprint_match = re.search(r'Sprint\s+(\d+)', content)
                    if sprint_match:
                        metadata['sprint'] = int(sprint_match.group(1))
        
        except Exception as e:
            print(f"Warning: Could not extract metadata from {test_file}: {e}")
        
        return metadata

class TestRunner:
    """Handles test compilation and execution"""
    
    def __init__(self, build_dir: str = "build/tests"):
        self.build_dir = Path(build_dir)
        self.build_dir.mkdir(parents=True, exist_ok=True)
        
    def compile_test(self, test_file: str, include_coverage: bool = False) -> Tuple[bool, str]:
        """Compile a single test file"""
        test_path = Path(test_file)
        output_name = self.build_dir / test_path.stem
        
        # Base compilation flags
        cflags = [
            "-Wall", "-Wextra", "-Werror", "-std=c99", "-g",
            "-Isrc", "-Itests", "-Itests/vendor"
        ]
        
        # Add coverage flags if requested
        if include_coverage:
            cflags.extend(["--coverage", "-O0"])
        
        # Libraries
        libs = ["-lm"]
        if sys.platform == "darwin":
            libs.extend(["-framework", "IOKit", "-framework", "CoreFoundation"])
        
        # Compile command
        cmd = ["clang"] + cflags + [test_file, "tests/vendor/unity.c"] + libs + ["-o", str(output_name)]
        
        try:
            result = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
            if result.returncode == 0:
                return True, str(output_name)
            else:
                return False, result.stderr
        except subprocess.TimeoutExpired:
            return False, "Compilation timeout"
        except Exception as e:
            return False, f"Compilation error: {e}"
    
    def run_test(self, executable: str) -> Tuple[bool, str, float]:
        """Run a compiled test and return results"""
        try:
            start_time = time.time()
            result = subprocess.run([executable], capture_output=True, text=True, timeout=60)
            duration = time.time() - start_time
            
            success = result.returncode == 0
            output = result.stdout + result.stderr
            
            return success, output, duration
        except subprocess.TimeoutExpired:
            return False, "Test execution timeout", 60.0
        except Exception as e:
            return False, f"Execution error: {e}", 0.0

class CoverageAnalyzer:
    """Handles code coverage analysis"""
    
    def __init__(self, src_dir: str = "src"):
        self.src_dir = Path(src_dir)
    
    def generate_coverage_report(self, test_executables: List[str]) -> Dict:
        """Generate coverage report after running tests"""
        coverage_data = {
            'total_lines': 0,
            'covered_lines': 0,
            'percentage': 0.0,
            'files': {}
        }
        
        try:
            # Run gcov on all source files
            for test_exe in test_executables:
                subprocess.run(['gcov', '-r', test_exe], 
                             capture_output=True, cwd=self.src_dir)
            
            # Parse gcov output files
            for gcov_file in self.src_dir.rglob("*.gcov"):
                self._parse_gcov_file(gcov_file, coverage_data)
            
            # Calculate total percentage
            if coverage_data['total_lines'] > 0:
                coverage_data['percentage'] = (
                    coverage_data['covered_lines'] / coverage_data['total_lines'] * 100
                )
        
        except Exception as e:
            print(f"Coverage analysis error: {e}")
        
        return coverage_data
    
    def _parse_gcov_file(self, gcov_file: Path, coverage_data: Dict):
        """Parse individual gcov file"""
        try:
            with open(gcov_file, 'r') as f:
                lines = f.readlines()
            
            file_coverage = {'lines': 0, 'covered': 0}
            
            for line in lines:
                if line.strip() and not line.startswith('//'):
                    parts = line.split(':', 2)
                    if len(parts) >= 2:
                        execution_count = parts[0].strip()
                        if execution_count.isdigit() or execution_count == '#####':
                            file_coverage['lines'] += 1
                            if execution_count != '#####' and execution_count != '0':
                                file_coverage['covered'] += 1
            
            # Update global coverage data
            filename = gcov_file.stem.replace('.gcov', '')
            coverage_data['files'][filename] = file_coverage
            coverage_data['total_lines'] += file_coverage['lines']
            coverage_data['covered_lines'] += file_coverage['covered']
        
        except Exception as e:
            print(f"Error parsing {gcov_file}: {e}")

def run_cloc_analysis():
    """Run cloc analysis on the codebase"""
    try:
        result = subprocess.run(['cloc', 'src/', '--json'], 
                              capture_output=True, text=True)
        if result.returncode == 0:
            return json.loads(result.stdout)
    except Exception as e:
        print(f"CLOC analysis failed: {e}")
    return None

def main():
    parser = argparse.ArgumentParser(description='CGame Test Automation')
    parser.add_argument('--category', choices=['unit', 'integration', 'performance', 'regression', 'backlog', 'all'], 
                       default='all', help='Test category to run')
    parser.add_argument('--coverage', action='store_true', help='Generate coverage report')
    parser.add_argument('--parallel', type=int, default=1, help='Number of parallel test processes')
    parser.add_argument('--verbose', action='store_true', help='Verbose output')
    parser.add_argument('--file', help='Run specific test file')
    
    args = parser.parse_args()
    
    # Initialize components
    discovery = TestDiscovery()
    runner = TestRunner()
    coverage = CoverageAnalyzer() if args.coverage else None
    
    print("🔍 CGame Test Suite Automation")
    print("=" * 50)
    
    # Discover tests
    discovered_tests = discovery.discover_tests()
    
    if args.file:
        # Run specific file
        test_files = [args.file]
        category = 'specific'
    elif args.category == 'all':
        # Run all tests
        test_files = []
        for cat_tests in discovered_tests.values():
            test_files.extend(cat_tests)
        category = 'all'
    else:
        # Run specific category
        test_files = discovered_tests.get(args.category, [])
        category = args.category
    
    if not test_files:
        print(f"❌ No tests found for category: {category}")
        return 1
    
    print(f"📋 Running {len(test_files)} tests in category: {category}")
    
    # Run tests
    results = {
        'passed': 0,
        'failed': 0,
        'total_time': 0.0,
        'failures': []
    }
    
    compiled_executables = []
    
    for test_file in test_files:
        if args.verbose:
            print(f"🔧 Compiling {test_file}...")
        
        # Compile test
        success, output = runner.compile_test(test_file, args.coverage)
        if not success:
            print(f"❌ Compilation failed for {test_file}")
            if args.verbose:
                print(f"   Error: {output}")
            results['failed'] += 1
            continue
        
        executable = output
        compiled_executables.append(executable)
        
        if args.verbose:
            print(f"🧪 Running {test_file}...")
        
        # Run test
        success, test_output, duration = runner.run_test(executable)
        results['total_time'] += duration
        
        if success:
            results['passed'] += 1
            if args.verbose:
                print(f"✅ {test_file} passed ({duration:.2f}s)")
        else:
            results['failed'] += 1
            results['failures'].append({
                'file': test_file,
                'output': test_output,
                'duration': duration
            })
            print(f"❌ {test_file} failed ({duration:.2f}s)")
            if args.verbose:
                print(f"   Output: {test_output}")
    
    # Generate coverage report
    if args.coverage and coverage and compiled_executables:
        print("\n📊 Generating coverage report...")
        coverage_data = coverage.generate_coverage_report(compiled_executables)
        print(f"📈 Coverage: {coverage_data['percentage']:.1f}% ({coverage_data['covered_lines']}/{coverage_data['total_lines']} lines)")
    
    # Generate CLOC report
    cloc_data = run_cloc_analysis()
    if cloc_data:
        c_stats = cloc_data.get('C', {})
        print(f"📏 Codebase: {c_stats.get('nFiles', 0)} files, {c_stats.get('code', 0)} lines of code")
    
    # Summary
    print("\n" + "=" * 50)
    print(f"📊 Test Results Summary")
    print(f"   Passed: {results['passed']}")
    print(f"   Failed: {results['failed']}")
    print(f"   Total Time: {results['total_time']:.2f}s")
    print(f"   Success Rate: {results['passed']/(results['passed']+results['failed'])*100:.1f}%")
    
    if results['failures']:
        print(f"\n❌ Failed Tests:")
        for failure in results['failures']:
            print(f"   - {failure['file']}")
    
    return 0 if results['failed'] == 0 else 1

if __name__ == "__main__":
    sys.exit(main())
