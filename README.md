# thread-synchronization

This project is about the classic readers-writers synchronization problem. Several threads wish to read and write data shared between them. In this scenario, multiple readers may concurrently access the data safely, without any correctness issue. However, a writer is not allowed to access the data concurrently with anyone else, either a reader or a writer. This is achieved by having special kinds of locks called reader-write locks which have separate lock/unlock functions, depending on whether the thread asking for a lock is a reader or writer. If one reader asks for a lock while another reader already has it, the second reader will also be granted a read lock (unlike in the case of a regular mutex), thus encouraging more concurrency in the application.

