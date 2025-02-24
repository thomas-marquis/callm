import React, { useState } from 'react';
import useSSE from './useSSE';
import MatrixList from './MatrixList';
import Matrix from './Matrix';

const App: React.FC = () => {
  const messages = useSSE('/api/events');
  const [selectedLabel, setSelectedLabel] = useState<string | null>(null);

  const selectedMatrix = messages.find((msg) => msg.label === selectedLabel)?.matrix || [];

  return (
    <div className="app">
      <MatrixList labels={messages.map((msg) => msg.label)} onSelect={setSelectedLabel} />
      <div className="matrix-visualization">
        {selectedLabel && <Matrix matrix={selectedMatrix} />}
      </div>
    </div>
  );
};

export default App;
