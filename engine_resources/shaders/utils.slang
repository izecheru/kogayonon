float4 colorFromNumber( uint32_t value )
{
  return float4( (float)( ( value >> 0 ) & 0xFF ) / 255.0f,  // R
                 (float)( ( value >> 8 ) & 0xFF ) / 255.0f,  // G
                 (float)( ( value >> 16 ) & 0xFF ) / 255.0f, // B
                 1.0f                                        // A
  );
}

