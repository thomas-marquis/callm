import { useEffect, useState } from 'react';

interface MatrixMessage {
  label: string;
  matrix: number[][];
}

const useSSE = (url: string) => {
  const [messages, setMessages] = useState<MatrixMessage[]>([]);

  useEffect(() => {
    const eventSource = new EventSource(url);

    eventSource.onmessage = (event) => {
      const data = JSON.parse(event.data);
      setMessages((prevMessages) => [...prevMessages, data]);
    };

    eventSource.onerror = () => {
      console.error('SSE connection error');
      eventSource.close();
    };

    return () => {
      eventSource.close();
    };
  }, [url]);

  return messages;
};

export default useSSE;
