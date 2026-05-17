uint4 colorFromNumber( uint32_t value )
{
  return uint4( ( value & 0xFF ) / 255,           // R
                ( ( value >> 8 ) & 0xFF ) / 255,  // G
                ( ( value >> 16 ) & 0xFF ) / 255, // B
                1                                 // A
  );
}

