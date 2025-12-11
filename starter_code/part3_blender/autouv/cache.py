import hashlib
import time 
import bpy
import numpy as np

# In-memory cache: {hash_key: (uv_data, metrics, timestamp)}
_uv_cache = {}
CACHE_TTL = 300  # Seconds to keep cache

def get_mesh_hash(mesh, params):
    # Geometry Hash (Vertices count + Edge selection for seams)
    v_count = len(mesh.vertices)
    e_count = len(mesh.edges)
    
    seam_edges = [e.index for e in mesh.edges if e.use_seam]
    geo_str = f"v{v_count}_e{e_count}_s{len(seam_edges)}_{sum(seam_edges)}"

    param_str = f"{params['angle_threshold']}_{params['min_island_faces']}_{params['island_margin']}"


    full_str = f"{geo_str}|{param_str}"
    return hashlib.md5(full_str.encode()).hexdigest()

def get_cached_result(mesh, params):
    cleanup_cache()
    key = get_mesh_hash(mesh, params)
    if key in _uv_cache:
        data = _uv_cache[key]
        _uv_cache[key] = (data[0], data[1], time.time())
        return data[0], data[1] # UVs, Metrics
    return None, None

def store_cached_result(mesh, params, uvs, metrics):
    key = get_mesh_hash(mesh, params)
    _uv_cache[key] = (uvs, metrics, time.time())

def cleanup_cache():
    now = time.time()
    expired = [k for k, v in _uv_cache.items() if now - v[2] > CACHE_TTL]
    for k in expired:
        del _uv_cache[k]

def clear_all():
    _uv_cache.clear()

def register(): pass
def unregister(): pass