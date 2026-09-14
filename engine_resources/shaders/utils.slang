uint4 colorFromNumber( uint32_t value )
{
  return uint4( ( value & 0xFF ) / 255,           // R
                ( ( value >> 8 ) & 0xFF ) / 255,  // G
                ( ( value >> 16 ) & 0xFF ) / 255, // B
                1                                 // A
  );
}

float opacityFalloff( float3 coords, float3 cameraPos, float maxFadeDistance )
{
  float distanceToCamera = length( coords - cameraPos );
  return smoothstep( 1.0, 0.0, distanceToCamera / maxFadeDistance );
}

float4x4 inverse( float4x4 m )
{
  // Extract rows for easier computation
  float4 a0 = m[0];
  float4 a1 = m[1];
  float4 a2 = m[2];
  float4 a3 = m[3];

  // Compute cofactors
  float4x4 inv;

  // First row of cofactors
  inv[0][0] = +( a1.y * ( a2.z * a3.w - a2.w * a3.z ) - a1.z * ( a2.y * a3.w - a2.w * a3.y ) +
                 a1.w * ( a2.y * a3.z - a2.z * a3.y ) );
  inv[0][1] = -( a1.x * ( a2.z * a3.w - a2.w * a3.z ) - a1.z * ( a2.x * a3.w - a2.w * a3.x ) +
                 a1.w * ( a2.x * a3.z - a2.z * a3.x ) );
  inv[0][2] = +( a1.x * ( a2.y * a3.w - a2.w * a3.y ) - a1.y * ( a2.x * a3.w - a2.w * a3.x ) +
                 a1.w * ( a2.x * a3.y - a2.y * a3.x ) );
  inv[0][3] = -( a1.x * ( a2.y * a3.z - a2.z * a3.y ) - a1.y * ( a2.x * a3.z - a2.z * a3.x ) +
                 a1.z * ( a2.x * a3.y - a2.y * a3.x ) );

  // Second row of cofactors
  inv[1][0] = -( a0.y * ( a2.z * a3.w - a2.w * a3.z ) - a0.z * ( a2.y * a3.w - a2.w * a3.y ) +
                 a0.w * ( a2.y * a3.z - a2.z * a3.y ) );
  inv[1][1] = +( a0.x * ( a2.z * a3.w - a2.w * a3.z ) - a0.z * ( a2.x * a3.w - a2.w * a3.x ) +
                 a0.w * ( a2.x * a3.z - a2.z * a3.x ) );
  inv[1][2] = -( a0.x * ( a2.y * a3.w - a2.w * a3.y ) - a0.y * ( a2.x * a3.w - a2.w * a3.x ) +
                 a0.w * ( a2.x * a3.y - a2.y * a3.x ) );
  inv[1][3] = +( a0.x * ( a2.y * a3.z - a2.z * a3.y ) - a0.y * ( a2.x * a3.z - a2.z * a3.x ) +
                 a0.z * ( a2.x * a3.y - a2.y * a3.x ) );

  // Third row of cofactors
  inv[2][0] = +( a0.y * ( a1.z * a3.w - a1.w * a3.z ) - a0.z * ( a1.y * a3.w - a1.w * a3.y ) +
                 a0.w * ( a1.y * a3.z - a1.z * a3.y ) );
  inv[2][1] = -( a0.x * ( a1.z * a3.w - a1.w * a3.z ) - a0.z * ( a1.x * a3.w - a1.w * a3.x ) +
                 a0.w * ( a1.x * a3.z - a1.z * a3.x ) );
  inv[2][2] = +( a0.x * ( a1.y * a3.w - a1.w * a3.y ) - a0.y * ( a1.x * a3.w - a1.w * a3.x ) +
                 a0.w * ( a1.x * a3.y - a1.y * a3.x ) );
  inv[2][3] = -( a0.x * ( a1.y * a3.z - a1.z * a3.y ) - a0.y * ( a1.x * a3.z - a1.z * a3.x ) +
                 a0.z * ( a1.x * a3.y - a1.y * a3.x ) );

  // Fourth row of cofactors
  inv[3][0] = -( a0.y * ( a1.z * a2.w - a1.w * a2.z ) - a0.z * ( a1.y * a2.w - a1.w * a2.y ) +
                 a0.w * ( a1.y * a2.z - a1.z * a2.y ) );
  inv[3][1] = +( a0.x * ( a1.z * a2.w - a1.w * a2.z ) - a0.z * ( a1.x * a2.w - a1.w * a2.x ) +
                 a0.w * ( a1.x * a2.z - a1.z * a2.x ) );
  inv[3][2] = -( a0.x * ( a1.y * a2.w - a1.w * a2.y ) - a0.y * ( a1.x * a2.w - a1.w * a2.x ) +
                 a0.w * ( a1.x * a2.y - a1.y * a2.x ) );
  inv[3][3] = +( a0.x * ( a1.y * a2.z - a1.z * a2.y ) - a0.y * ( a1.x * a2.z - a1.z * a2.x ) +
                 a0.z * ( a1.x * a2.y - a1.y * a2.x ) );

  // Compute determinant
  float det = a0.x * inv[0][0] + a0.y * inv[0][1] + a0.z * inv[0][2] + a0.w * inv[0][3];

  // Check for singularity
  if ( abs( det ) < 1e-8 )
  {
    return float4x4( 0 ); // Return zero matrix for singular case
  }

  // Divide by determinant and transpose to get inverse
  float invDet = 1.0 / det;
  return float4x4( inv[0][0] * invDet,
                   inv[1][0] * invDet,
                   inv[2][0] * invDet,
                   inv[3][0] * invDet,
                   inv[0][1] * invDet,
                   inv[1][1] * invDet,
                   inv[2][1] * invDet,
                   inv[3][1] * invDet,
                   inv[0][2] * invDet,
                   inv[1][2] * invDet,
                   inv[2][2] * invDet,
                   inv[3][2] * invDet,
                   inv[0][3] * invDet,
                   inv[1][3] * invDet,
                   inv[2][3] * invDet,
                   inv[3][3] * invDet );
}

float3x3 inverse( float3x3 m )
{
  // Extract rows for easier computation
  float3 a0 = m[0];
  float3 a1 = m[1];
  float3 a2 = m[2];

  // Compute cofactors
  float3x3 inv;

  // First row of cofactors
  inv[0][0] = +( a1.y * a2.z - a1.z * a2.y );
  inv[0][1] = -( a1.x * a2.z - a1.z * a2.x );
  inv[0][2] = +( a1.x * a2.y - a1.y * a2.x );

  // Second row of cofactors
  inv[1][0] = -( a0.y * a2.z - a0.z * a2.y );
  inv[1][1] = +( a0.x * a2.z - a0.z * a2.x );
  inv[1][2] = -( a0.x * a2.y - a0.y * a2.x );

  // Third row of cofactors
  inv[2][0] = +( a0.y * a1.z - a0.z * a1.y );
  inv[2][1] = -( a0.x * a1.z - a0.z * a1.x );
  inv[2][2] = +( a0.x * a1.y - a0.y * a1.x );

  // Compute determinant
  float det = a0.x * inv[0][0] + a0.y * inv[0][1] + a0.z * inv[0][2];

  // Check for singularity
  if ( abs( det ) < 1e-8 )
  {
    return float3x3( 0 ); // Return zero matrix for singular case
  }

  // Divide by determinant and transpose to get inverse
  float invDet = 1.0 / det;
  return float3x3( inv[0][0] * invDet,
                   inv[1][0] * invDet,
                   inv[2][0] * invDet,
                   inv[0][1] * invDet,
                   inv[1][1] * invDet,
                   inv[2][1] * invDet,
                   inv[0][2] * invDet,
                   inv[1][2] * invDet,
                   inv[2][2] * invDet );
}

