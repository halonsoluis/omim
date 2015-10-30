attribute vec4 a_position;
attribute vec2 a_normal;
attribute vec2 a_colorTexCoord;
attribute vec2 a_outlineColorTexCoord;
attribute vec2 a_maskTexCoord;

uniform mat4 modelView;
uniform mat4 projection;
uniform mat4 pivotTransform;

varying vec2 v_colorTexCoord;
varying vec2 v_maskTexCoord;
varying vec2 v_outlineColorTexCoord;

void main()
{
  // Here we intentionally decrease precision of 'pivot' calculation
  // to eliminate jittering effect in process of billboard reconstruction.
  lowp vec4 pivot = a_position * modelView;
  vec4 offset = vec4(a_normal, 0, 0) * projection;

  vec4 projectedPivot = pivot * projection;
  vec4 transformedPivot = pivotTransform * vec4(projectedPivot.xy, 0, 1);

  vec4 scale = pivotTransform * vec4(1.0, -1.0, 0, 1.0);
  gl_Position = transformedPivot + vec4(offset.xy * transformedPivot.w / scale.w * scale.x, 0, 0);

  v_colorTexCoord = a_colorTexCoord;
  v_maskTexCoord = a_maskTexCoord;
  v_outlineColorTexCoord = a_outlineColorTexCoord;
}
