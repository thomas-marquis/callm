import React from 'react';
import ProbeMessage from "../entities/ProbeMessage";
import { List, ListItemButton, ListItemText } from '@mui/material';

interface MatrixListProps {
    messages: ProbeMessage[];
    onSelect: (label: string) => void;
    style?: React.CSSProperties;
}

const MatrixList: React.FC<MatrixListProps> = ({ messages, onSelect, style }) => {
    return (
        <List style={style}>
            {
                messages.map((msg, i) => (
                    <ListItemButton style={{ minWidth: '200px' }} key={`${msg.label}-${i}`} onClick={() => onSelect(msg.label)}>
                        <ListItemText primary={`${msg.label} ${msg.matrix.length} X ${msg.matrix[0].length}`} />
                    </ListItemButton>
                ))
            }
        </List >
    );
};

export default MatrixList;
