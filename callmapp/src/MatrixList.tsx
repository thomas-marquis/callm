import React from 'react';

interface MatrixListProps {
  labels: string[];
  onSelect: (label: string) => void;
}

const MatrixList: React.FC<MatrixListProps> = ({ labels, onSelect }) => {
  return (
    <div className="matrix-list">
      <h2>Matrices</h2>
      <ul>
        {labels.map((label, index) => (
          <li key={index} onClick={() => onSelect(label)}>
            {label}
          </li>
        ))}
      </ul>
    </div>
  );
};

export default MatrixList;
