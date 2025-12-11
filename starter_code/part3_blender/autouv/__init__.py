bl_info = {
    "name": "Auto UV Unwrap",
    "author": "Your Name",
    "version": (1, 0),
    "blender": (3, 0, 0),
    "location": "View3D > Sidebar > AutoUV",
    "description": "Automatic UV unwrapping with LSCM, caching, and live preview",
    "category": "UV",
}

import bpy
import sys
import os

# Ensure Blender can find the 'part2_python' library
current_dir = os.path.dirname(os.path.abspath(__file__))
# Navigate up: autouv -> part3_blender -> root
project_root = os.path.dirname(os.path.dirname(current_dir))
python_lib_path = os.path.join(project_root, 'part2_python')

if python_lib_path not in sys.path:
    sys.path.append(python_lib_path)

# Import submodules
from . import operator
from . import panel
from . import cache
from . import live_preview
from . import seam_tools

def register():
    cache.register()
    operator.register()
    seam_tools.register()
    live_preview.register()
    panel.register()

def unregister():
    panel.unregister()
    live_preview.unregister()
    seam_tools.unregister()
    operator.unregister()
    cache.unregister()

if __name__ == "__main__":
    register()