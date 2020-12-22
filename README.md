# MLFQ
MLFQ C语言实现
MLFQ规则如下：

• Rule 1: If Priority(A) > Priority(B), A runs (B doesn’t).<br>
• Rule 2: If Priority(A) = Priority(B), A & B run in round-robin fash-
ion using the time slice (quantum length) of the given queue.<br>
• Rule 3: When a job enters the system, it is placed at the highest
priority (the topmost queue).<br>
• Rule 4: Once a job uses up its time allotment at a given level (re-
gardless of how many times it has given up the CPU), its priority is
reduced (i.e., it moves down one queue).<br>
• Rule 5: After some time period S, move all the jobs in the system
to the topmost queue.<br>
