import bpy

class AutoUV_OT_MarkSeam(bpy.types.Operator):
    """Marking selected edges as seams"""
    bl_idname = "autouv.mark_seam"
    bl_label = "Mark Seam"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        bpy.ops.mesh.mark_seam(clear=False)
        # Note: Cache invalidation happens automatically because the
        # geometry hash (which includes seam edges) will change.
        return {'FINISHED'} # type: ignore
    
class AutoUV_OT_ClearSeam(bpy.types.Operator):
    """Clear seams from selected edges"""
    bl_idname = "autouv.clear_seam"
    bl_label = "Clear Seam"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        bpy.ops.mesh.mark_seam(clear=True)
        return {'FINISHED'} # type: ignore

def register():
    bpy.utils.register_class(AutoUV_OT_MarkSeam)
    bpy.utils.register_class(AutoUV_OT_ClearSeam)

def unregister():
    bpy.utils.unregister_class(AutoUV_OT_MarkSeam)
    bpy.utils.unregister_class(AutoUV_OT_ClearSeam)