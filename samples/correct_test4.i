loadI	1024	=>	r0
loadI	1024	=>	r19
loadI	1028	=>	r18
load	r19	=>	r17				// a = [1024]
load	r18	=>	r19				// b = [1028]
loadI	1032	=>	r18	
loadI	1036	=>	r16
loadI	1040	=>	r15
loadI	1044	=>	r14
store	r17	=>	r18				// [1032] = a
add	r17,	r19	=>	r13			// a + b
store	r13	=>	r16				// a + b => [1036]
add	r13,	r19	=>	r17			// a + b + b
store	r17	=>	r15				// a + b + b => [1040]
store	r19	=>	r14				// b => [1044]
load	r18	=>	r17				// a
load	r16	=>	r18				// a + b
load	r15	=>	r16				// a + b + b
lshift	r17,	r19	=>	r15		// a << b
mult	r15,	r18	=>	r19		// (a << b) * (a + b) {4}
load	r14	=>	r18				// b
mult	r19,	r16	=>	r14		// (a << b) * (a + b) * (a + b + b)
mult	r14,	r18	=>	r16		// (a << b) * (a + b) * (a + b + b) * b
loadI	1032	=>	r18			
store	r16	=>	r18
output	1032
