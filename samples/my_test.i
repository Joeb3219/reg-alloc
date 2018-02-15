loadI	1024 	=> r0
loadI	1	=> r1
loadI	2	=> r2
loadI	3	=> r3
loadI	4	=> r4
loadI	5	=> r5
add	r1, r2	=> r6
add	r3, r6	=> r7
storeAI	r7	=> r0, 4
outputAI	r0, 4
