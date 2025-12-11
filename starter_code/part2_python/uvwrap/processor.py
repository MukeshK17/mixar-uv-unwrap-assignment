"""
Multi-threaded batch processor

TEMPLATE - YOU IMPLEMENT

Processes multiple meshes in parallel using ThreadPoolExecutor.
"""

from concurrent.futures import ThreadPoolExecutor, as_completed
import threading
import time
import os
from pathlib import Path
import numpy as np
import sys

try:
    from .bindings import load_mesh, save_mesh, unwrap
    from .metrics import compute_stretch, compute_coverage, compute_angle_distortion
except ImportError:
    sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    from uvwrap.bindings import load_mesh, unwrap, save_mesh
    from uvwrap.metrics import compute_stretch, compute_coverage, compute_angle_distortion


class UnwrapProcessor:
    """
    Multi-threaded UV unwrapping batch processor

    Example usage:
        processor = UnwrapProcessor(num_threads=8)
        results = processor.process_batch(
            input_files=["mesh1.obj", "mesh2.obj"],
            output_dir="output/",
            params={'angle_threshold': 30.0},
            on_progress=lambda cur, total, name: print(f"{cur}/{total}")
        )
    """

    def __init__(self, num_threads=None):
        """
        Initialize processor

        Args:
            num_threads: Number of worker threads (default: CPU count)
        """
        self.num_threads = num_threads or os.cpu_count()
        self.progress_lock = threading.Lock()
        self.completed = 0

    def process_batch(self, input_files, output_dir, params,
                     on_progress=None):
        """
        Process multiple meshes in parallel

        Args:
            input_files: List of input file paths
            output_dir: Output directory
            params: Dictionary of unwrap parameters
            on_progress: Optional callback(current, total, filename)

        Returns:
            Dictionary with results:
            {
                'summary': {
                    'total': int,
                    'success': int,
                    'failed': int,
                    'total_time': float,
                    'avg_time': float,
                    'avg_stretch': float,
                    'avg_coverage': float,
                },
                'files': [
                    {
                        'file': str,
                        'vertices': int,
                        'triangles': int,
                        'time': float,
                        'metrics': {...}
                    },
                    ...
                ]
            }

        IMPLEMENTATION REQUIRED
        """
        # TODO: Implement
        #
        # Steps:
        # 1. Create output directory
        # 2. Create ThreadPoolExecutor with num_threads
        # 3. Submit all tasks
        # 4. Collect results as they complete
        # 5. Update progress (thread-safe!)
        # 6. Compute summary statistics
        # 7. Return results

        os.makedirs(output_dir, exist_ok=True)
        start_time = time.time()
        results = []
        total = len(input_files)
        self.completed = 0

        # TODO: YOUR CODE HERE
        futures_map = {}
        print(f"Starting batch processing... {total} files using {self.num_threads} threads.")

        with ThreadPoolExecutor(max_workers=self.num_threads) as executor:
            # 3.
            for f_path in input_files:
                future = executor.submit(self._process_single, f_path, output_dir, params)
                futures_map[future] = f_path

            # 4.
            for future in as_completed(futures_map):
                original_file = futures_map[future]
                
                try:
                    res = future.result()
                    results.append(res)
                except Exception as e:
                    results.append({
                        'file': os.path.basename(original_file),
                        'error': str(e),
                        'success': False,
                        'time': 0.0
                    })

                with self.progress_lock:
                    self.completed += 1
                    current_count = self.completed
                
                if on_progress:
                    on_progress(current_count, total, os.path.basename(original_file))

        total_time = time.time() - start_time
        summary = self._compute_summary(results, total_time) 

        return {
            'summary': self._compute_summary(results, total_time),
            'files': results
        }

    def _process_single(self, input_path, output_dir, params):
        """
        Process single file (runs in worker thread)

        Args:
            input_path: Input file path
            output_dir: Output directory
            params: Unwrap parameters

        Returns:
            Result dictionary

        IMPLEMENTATION REQUIRED
        """

        file_name = os.path.basename(input_path)
        out_path = os.path.join(output_dir, file_name)
        t0 = time.time()
        try:
        # TODO: Implement
        #
        # Steps:
        # 1. Load mesh
        # 2. Unwrap with params
        # 3. Compute metrics
        # 4. Save to output_dir
        # 5. Return result dict with:
        #      - file name
        #      - vertex/triangle counts
        #      - time elapsed
        #      - quality metrics

        # pass  # YOUR CODE HERE
            mesh = load_mesh(input_path)
            unwrapped_mesh, unwrap_res = unwrap(mesh, params)

            stretch = compute_stretch(unwrapped_mesh, unwrapped_mesh.uvs)
            coverage = compute_coverage(unwrapped_mesh.uvs, unwrapped_mesh.triangles)
            angle_dist = compute_angle_distortion(unwrapped_mesh, unwrapped_mesh.uvs)

            metrics = {
                'stretch': stretch,
                'coverage': coverage,
                'angle_distortion': angle_dist,
                'num_islands': unwrap_res.get('num_islands', 0)
            }


            save_mesh(unwrapped_mesh, out_path)
            duration = time.time() - t0
            return {
                'file': file_name,
                'vertices': mesh.num_vertices,
                'triangles': mesh.num_triangles,
                'time': duration,
                'metrics': metrics,
                'success': True
            }
        
        except Exception as e:
            raise RuntimeError(f"Error processing {file_name}: {str(e)}")
        

    def _compute_summary(self, results, total_time):
        """
        Compute summary statistics

        Args:
            results: List of result dictionaries
            total_time: Total elapsed time

        Returns:
            Summary dictionary
        """
        # TODO: Implement
        #
        # Compute:
        # - Total files
        # - Successful/failed counts
        # - Average time per file
        # - Average stretch
        # - Average coverage

        # pass  # YOUR CODE HERE
        total = len(results)
        successful = [r for r in results if r.get('success', False)]
        failed = [r for r in results if not r.get('success', False)]

        num_success = len(successful)

        if num_success > 0:
            avg_time = sum(r['time'] for r in successful) / num_success
            avg_stretch = sum(r['metrics']['stretch'] for r in successful) / num_success
            avg_coverage = sum(r['metrics']['coverage'] for r in successful) / num_success
            avg_angle = sum(r['metrics']['angle_distortion'] for r in successful) / num_success
        else:
            avg_time = 0.0
            avg_stretch = 0.0
            avg_coverage = 0.0
            avg_angle = 0.0

        return {
            'total_files': total,
            'successful': num_success,
            'failed': len(failed),
            'total_real_time': total_time,
            'avg_processing_time': avg_time,
            'avg_stretch': avg_stretch,
            'avg_coverage': avg_coverage,
            'avg_angle_distortion': avg_angle
        }

# Example usage
if __name__ == "__main__":
    # Test batch processing
    processor = UnwrapProcessor(num_threads=4)

    input_dir = 'test_input'
    output_dir = 'test_output'
    os.makedirs(input_dir, exist_ok=True)
    for i in range(5):
        Path(f"{input_dir}/mesh_{i}.obj").touch()

    processor = UnwrapProcessor(num_threads=2)


    def progress_callback(current, total, filename):
        pct = int(100 * current / total)
        print(f"\r[{current}/{total}] {pct}% - {filename}", end='', flush=True)

    # TODO: Test with real files
    # results = processor.process_batch(
    #     input_files=[...],
    #     output_dir="output/",
    #     params={'angle_threshold': 30.0},
    #     on_progress=progress_callback
    # )
    # print(f"\nCompleted: {results['summary']}")
    # pass
    print("\nstarting batch...")
    try:
        results = processor.process_batch(
            input_files=[os.path.join(input_dir, f) for f in os.listdir(input_dir) if f.endswith('.obj')],
            output_dir=output_dir,
            params={'angle_threshold': 30.0},
            on_progress=progress_callback
        )
        print(f"\n\nBatch Complete.")
        print(f"Summary: {results['summary']}")
        
    except Exception as e:
        print(f"\nBatch failed: {e}")
        
