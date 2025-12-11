import bpy
import bmesh
import numpy as np
import time

# Import your Part 2 implementation
from uvwrap.bindings import Mesh, unwrap
from uvwrap.metrics import compute_stretch, compute_coverage, compute_angle_distortion
from . import cache

class AutoUV_OT_Unwrap(bpy.types.Operator):
    """Unwrap active mesh (uses caching)"""
    bl_idname = "autouv.unwrap"
    bl_label = "Auto UV Unwrap"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        obj = context.active_object
        if not obj or obj.type != 'MESH':
            self.report({'ERROR'}, "Select a mesh object")
            return {'CANCELLED'}

        self.unwrap_object(context, obj)
        return {'FINISHED'}

    def unwrap_object(self, context, obj):
        scene = context.scene
        params = {
            'angle_threshold': scene.autouv_angle_threshold,
            'min_island_faces': scene.autouv_min_island,
            'island_margin': scene.autouv_margin,
            'pack_islands': True
        }

        # 1. Check Cache
        cached_uvs, cached_metrics = cache.get_cached_result(obj.data, params)
        if cached_uvs is not None:
            self.report({'INFO'}, "Restored from Cache")
            self.apply_uvs(obj.data, cached_uvs)
            self.update_ui_metrics(scene, cached_metrics)
            return

        # 2. Extract Data
        # Ensure we are in object mode to read latest data
        original_mode = obj.mode
        if obj.mode != 'OBJECT':
            bpy.ops.object.mode_set(mode='OBJECT')

        bm = bmesh.new()
        bm.from_mesh(obj.data)
        bmesh.ops.triangulate(bm, faces=bm.faces)
        
        verts = np.array([v.co for v in bm.verts], dtype=np.float32)
        bm.verts.ensure_lookup_table()
        tris = np.array([[v.index for v in f.verts] for f in bm.faces], dtype=np.int32)
        bm.free()

        # Restore mode
        if original_mode != 'OBJECT':
            bpy.ops.object.mode_set(mode=original_mode)

        if len(tris) == 0: return

        # 3. Process
        input_mesh = Mesh(verts, tris)
        try:
            result_mesh, res_stats = unwrap(input_mesh, params)
            
            # Metrics
            metrics = {
                'stretch': compute_stretch(result_mesh, result_mesh.uvs),
                'coverage': compute_coverage(result_mesh.uvs, result_mesh.triangles),
                'angle': compute_angle_distortion(result_mesh, result_mesh.uvs)
            }
            
            # 4. Cache & Apply
            cache.store_cached_result(obj.data, params, result_mesh.uvs, metrics)
            self.apply_uvs(obj.data, result_mesh.uvs)
            self.update_ui_metrics(scene, metrics)
            
        except Exception as e:
            self.report({'ERROR'}, f"Unwrap Failed: {e}")

    def apply_uvs(self, mesh_data, uvs):
        uv_layer = mesh_data.uv_layers.active or mesh_data.uv_layers.new(name="AutoUV")
        # Simple loop mapping (assuming 1-to-1 vertex correspondence for Mock Mode)
        for poly in mesh_data.polygons:
            for loop_index in poly.loop_indices:
                v_idx = mesh_data.loops[loop_index].vertex_index
                if v_idx < len(uvs):
                    uv_layer.data[loop_index].uv = uvs[v_idx]

    def update_ui_metrics(self, scene, metrics):
        scene['autouv_score_stretch'] = metrics['stretch']
        scene['autouv_score_coverage'] = metrics['coverage']
        scene['autouv_score_angle'] = np.degrees(metrics['angle'])

class AutoUV_OT_BatchUnwrap(bpy.types.Operator):
    """Unwrap all selected meshes"""
    bl_idname = "autouv.batch_unwrap"
    bl_label = "Batch Unwrap Selected"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        selected = [o for o in context.selected_objects if o.type == 'MESH']
        if not selected:
            self.report({'WARNING'}, "No meshes selected")
            return {'CANCELLED'}
        
        # Reuse single unwrap logic
        unwrapper = AutoUV_OT_Unwrap()
        
        # Progress Bar
        wm = context.window_manager
        wm.progress_begin(0, len(selected))
        
        for i, obj in enumerate(selected):
            wm.progress_update(i)
            # Set active for context
            context.view_layer.objects.active = obj
            unwrapper.unwrap_object(context, obj)
            
        wm.progress_end()
        self.report({'INFO'}, f"Batch Processed {len(selected)} objects")
        return {'FINISHED'}

def register():
    bpy.utils.register_class(AutoUV_OT_Unwrap)
    bpy.utils.register_class(AutoUV_OT_BatchUnwrap)

def unregister():
    bpy.utils.unregister_class(AutoUV_OT_Unwrap)
    bpy.utils.unregister_class(AutoUV_OT_BatchUnwrap)