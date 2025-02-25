import React, { useState } from 'react';
import useSSE from './useSSE';
import MatrixList from './components/MatrixList';
import Matrix from './components/Matrix';
import { Drawer, IconButton, Button, AppBar, Toolbar, Typography } from '@mui/material';
import MenuIcon from '@mui/icons-material/Menu';
import ProbeMessage from './entities/ProbeMessage';


const fakeLitleMessage: ProbeMessage = {
    label: 'fake little',
    matrix: [
        [1, 2],
        [3, 4],
    ],
};


const bigMat: number[][] = [];
for (let i = 0; i < 100; i++) {
    bigMat.push([]);
    for (let j = 0; j < 100; j++) {
        bigMat[i].push(Math.floor(Math.random() * 100));
    }
}

const fakeBigMessage: ProbeMessage = {
    label: 'fake big',
    matrix: bigMat,
};


const App: React.FC = () => {
    // const messages = useSSE('/api/events');
    const messages = [fakeLitleMessage, fakeBigMessage];
    const [selectedLabel, setSelectedLabel] = useState<string | null>(null);
    const [drawerOpen, setDrawerOpen] = useState(true);

    const selectedMatrix = messages.find((msg) => msg.label === selectedLabel)?.matrix || [];

    const toggleDrawer = () => {
        setDrawerOpen(!drawerOpen);
    };

    return (
        <div className="app">
            <AppBar position="static">
                <Toolbar>
                    <IconButton edge="start" color="inherit" aria-label="menu" onClick={toggleDrawer}>
                        <MenuIcon />
                    </IconButton>
                    <Typography variant="h6">
                        Matrix Visualization
                    </Typography>
                </Toolbar>
            </AppBar>
            <Drawer variant="persistent" anchor="left" open={drawerOpen} onClose={toggleDrawer}>
                <Button onClick={() => setDrawerOpen(false)} style={{ position: 'absolute', top: '20px', width: '100%' }}>
                    Fermer
                </Button>
                <MatrixList messages={messages} onSelect={setSelectedLabel} style={{ marginTop: '100px' }} />
            </Drawer>
            <div className="matrix-visualization" style={{ marginLeft: drawerOpen ? 240 : 0, transition: 'margin 0.3s' }}>
                {selectedLabel && <Matrix matrix={selectedMatrix} />}
            </div>
        </div>
    );
};

export default App;
